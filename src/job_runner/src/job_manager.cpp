#include "job_runner/job_manager.h"
#include "job_runner/job_runner.h"
#include <QCoreApplication>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QTimer>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QEventLoop>
#include <QFile>
#include <QDir>
#include <QHttpMultiPart>
#include <QUrlQuery>
#include <QDebug>
#include <QTextStream>

namespace providers_agent {
namespace job_runner {

class JobManager::Impl : public QObject {
    Q_OBJECT

public:
    Impl(const std::string& nodeId, const std::string& oauthToken)
        : nodeId_(QString::fromStdString(nodeId)), 
          oauthToken_(QString::fromStdString(oauthToken)),
          intervalSeconds_(10) {
        
        manager_ = new QNetworkAccessManager(this);
        timer_ = new QTimer(this);
        
        connect(timer_, &QTimer::timeout, this, &Impl::pollJobs);
        
        // Connect JobRunner signals
        connect(&jobRunner_, &JobRunner::readyReadStandardOutput, this, [this](const QString& output) {
            if (logCallback_) logCallback_("[Blender] " + output.trimmed().toStdString());
        });
        connect(&jobRunner_, &JobRunner::readyReadStandardError, this, [this](const QString& output) {
             if (logCallback_) logCallback_("[Blender ERR] " + output.trimmed().toStdString());
        });
        connect(&jobRunner_, &JobRunner::finished, this, &Impl::onBlenderFinished);

        // Ensure temp directory exists
        // QDir().mkpath("temp_jobs");
        
        // Use absolute path for temp_jobs relative to application dir (bin/temp_jobs)
        // This avoids CWD ambiguity.
        QDir appDir(QCoreApplication::applicationDirPath());
        tempDir_ = appDir.filePath("temp_jobs");
        QDir().mkpath(tempDir_);
        
        // Get API endpoint from env
        const char* env_endpoint = std::getenv("HEART_BEAT_API_ENDPOINT");
        baseUrl_ = env_endpoint ? QString(env_endpoint) : "http://localhost:3000";
    }

    void start() {
        if (!timer_->isActive()) {
            timer_->start(intervalSeconds_ * 1000);
            pollJobs(); // Run immediately
        }
    }

    void stop() {
        timer_->stop();
        // Also kill running job? For now, let it finish or it stays orphan if app closes.
    }

    void setInterval(uint32_t seconds) {
        intervalSeconds_ = seconds;
        if (timer_->isActive()) {
            timer_->start(intervalSeconds_ * 1000);
        }
    }

    void setLogCallback(std::function<void(const std::string&)> callback) {
        logCallback_ = callback;
    }

private slots:
    void pollJobs() {
        if (isProcessing_) return;

        QUrl url(baseUrl_ + "/api/providers/me/jobs");
        QUrlQuery query;
        query.addQueryItem("status", "queued"); 
        
        QNetworkRequest request(url);
        request.setRawHeader("Authorization", "Bearer " + oauthToken_.toUtf8());

        QNetworkReply* reply = manager_->get(request);
        connect(reply, &QNetworkReply::finished, this, [this, reply]() {
            handlePollResponse(reply);
            reply->deleteLater();
        });
    }

    void onBlenderFinished(int exitCode, QProcess::ExitStatus exitStatus) {
        if (!isProcessing_) return;

        qDebug() << "Blender finished with code:" << exitCode;
        if (logCallback_) logCallback_("Blender finished with code: " + std::to_string(exitCode));

        QString jobId = currentJobId_;
        
        // Remove input file
        // QString inputFile = "temp_jobs/" + jobId + "_input.blend";
        QString inputFile = QDir(tempDir_).filePath(jobId + "_input.blend");
        QFile::remove(inputFile);

        // Remove GPU script
        // QString gpuScript = "temp_jobs/" + jobId + "_gpu_setup.py";
        QString gpuScript = QDir(tempDir_).filePath(jobId + "_gpu_setup.py");
        QFile::remove(gpuScript);

        if (exitCode != 0 || exitStatus != QProcess::NormalExit) {
             updateJobStatus(jobId, "failed", "Blender exited with error code " + QString::number(exitCode));
             isProcessing_ = false;
             QTimer::singleShot(1000, this, &Impl::pollJobs);
             return;
        }

        // Check for output file
        // Blender usually adds frame number: outputPrefix + "0001.png"
        // Note: Check extension (default png).
        QString outputFile = currentOutputPrefix_ + "0001.png";
        if (!QFile::exists(outputFile)) {
             // Try scanning directory for any output? 
             // For now assume standard behavior.
             updateJobStatus(jobId, "failed", "Output file not generated");
             isProcessing_ = false;
             QTimer::singleShot(1000, this, &Impl::pollJobs);
             return;
        }

        // Upload Result
        if (logCallback_) logCallback_("Uploading result...");
        if (!uploadResult(jobId, outputFile)) {
             updateJobStatus(jobId, "failed", "Failed to upload result");
        } else {
             // if (logCallback_) logCallback_("Job " + jobId.toStdString() + " COMPLETED");
        }
        
        // Clean up output
        QFile::remove(outputFile);

        isProcessing_ = false;
        currentJobId_.clear();
        currentOutputPrefix_.clear();
        
        // Poll immediately for next job
        QTimer::singleShot(1000, this, &Impl::pollJobs);
    }

private:
    QString nodeId_;
    QString oauthToken_;
    QString baseUrl_;
    uint32_t intervalSeconds_;
    std::function<void(const std::string&)> logCallback_;
    
    QNetworkAccessManager* manager_;
    QTimer* timer_;
    JobRunner jobRunner_;
    bool isProcessing_ = false;

    // Current Job State
    QString currentJobId_;
    QString currentOutputPrefix_;
    QString tempDir_;

    void handlePollResponse(QNetworkReply* reply) {
        if (reply->error() != QNetworkReply::NoError) {
            qWarning() << "Poll jobs failed:" << reply->errorString();
            return;
        }

        QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
        QJsonObject root = doc.object();
        
        if (!root["success"].toBool()) {
            qWarning() << "API returned error:" << root["error"].toString();
            return;
        }

        QJsonArray jobs = root["jobs"].toArray();
        for (const auto& j : jobs) {
            QJsonObject job = j.toObject();
            if (job["status"].toString() == "queued") {
                isProcessing_ = true;
                processJob(job);
                break; // One at a time
            }
        }
    }

    void processJob(const QJsonObject& job) {
        QString jobId = job["id"].toString();
        QString configRef = job["config_ref"].toString(); // Input file path
        
        qDebug() << "Processing Job:" << jobId;
        if (logCallback_) logCallback_("Starting Job: " + jobId.toStdString());

        // 1. Update status to running
        if (!updateJobStatus(jobId, "running")) {
            isProcessing_ = false;
            return;
        }

        // 2. Download file
        // QString inputFile = "temp_jobs/" + jobId + "_input.blend";
        QString inputFile = QDir(tempDir_).filePath(jobId + "_input.blend");
        QString downloadUrl = baseUrl_ + configRef; 
        
        if (logCallback_) logCallback_("Downloading input file...");
        if (!downloadFile(downloadUrl, inputFile)) {
            updateJobStatus(jobId, "failed", "Failed to download input file");
            isProcessing_ = false;
            return;
        }

        // 3. Create GPU Setup Script
        // QString gpuScriptPath = "temp_jobs/" + jobId + "_gpu_setup.py";
        QString gpuScriptPath = QDir(tempDir_).filePath(jobId + "_gpu_setup.py");
        if (!createGpuSetupScript(gpuScriptPath)) {
            qWarning() << "Failed to create GPU setup script, falling back to default/CPU";
        }

        // 4. Run Blender (Async)
        currentJobId_ = jobId;
        // currentOutputPrefix_ = "temp_jobs/" + jobId + "_out_";
        currentOutputPrefix_ = QDir(tempDir_).filePath(jobId + "_out_");
        QStringList args;
        args << "-b" << inputFile;
        
        // Add GPU script if created successfully
        if (QFile::exists(gpuScriptPath)) {
            args << "-P" << gpuScriptPath;
        }
        
        args << "-o" << currentOutputPrefix_ << "-f" << "1"; 

        qDebug() << "Starting Blender with args:" << args;
        if (logCallback_) logCallback_("Launching Blender...");
        
        if (!jobRunner_.startBlender(args)) {
             updateJobStatus(jobId, "failed", "Failed to start Blender");
             isProcessing_ = false;
             currentJobId_.clear();
             currentOutputPrefix_.clear();
             return;
        }
        
        // Now we return and wait for finished signal
    }

    bool createGpuSetupScript(const QString& path) {
        QFile file(path);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            return false;
        }

        QTextStream out(&file);
        out << "import bpy\n\n"
            << "def enable_gpu():\n"
            << "    print('[GPU-SETUP] Attempting to enable GPU rendering...')\n"
            << "    try:\n"
            << "        prop = bpy.context.preferences.addons['cycles'].preferences\n"
            << "        prop.refresh_devices()\n"
            << "        \n"
            << "        # Priority: OPTIX > CUDA > HIP > METAL > ONEAPI\n"
            << "        device_types = ['OPTIX', 'CUDA', 'HIP', 'METAL', 'ONEAPI']\n"
            << "        found = False\n"
            << "        \n"
            << "        for device_type in device_types:\n"
            << "            devices = prop.get_devices_for_type(device_type)\n"
            << "            if devices:\n"
            << "                print(f'[GPU-SETUP] Found {device_type} devices:')\n"
            << "                for d in devices:\n"
            << "                    print(f' - {d.name}')\n"
            << "                    d.use = True\n"
            << "                prop.compute_device_type = device_type\n"
            << "                found = True\n"
            << "                break\n"
            << "        \n"
            << "        if found:\n"
            << "            bpy.context.scene.cycles.device = 'GPU'\n"
            << "            print('[GPU-SETUP] GPU rendering enabled.')\n"
            << "        else:\n"
            << "            print('[GPU-SETUP] No compatible GPU devices found. Falling back to CPU.')\n"
            << "            bpy.context.scene.cycles.device = 'CPU'\n"
            << "    except Exception as e:\n"
            << "        print(f'[GPU-SETUP] Error enabling GPU: {e}')\n\n"
            << "try:\n"
            << "    enable_gpu()\n"
            << "except Exception as e:\n"
            << "    print(f'[GPU-SETUP] Script failed: {e}')\n";
            
        file.close();
        return true;
    }

    bool updateJobStatus(const QString& jobId, const QString& status, const QString& reason = "") {
        QUrl url(baseUrl_ + "/api/jobs/" + jobId + "/status");
        QNetworkRequest request(url);
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
        request.setRawHeader("Authorization", "Bearer " + oauthToken_.toUtf8());

        QJsonObject json;
        json["status"] = status;
        if (!reason.isEmpty()) json["failure_reason"] = reason;

        QNetworkReply* reply = manager_->post(request, QJsonDocument(json).toJson());
        
        // Synchronous wait (short)
        QEventLoop loop;
        connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
        loop.exec();
        
        bool success = (reply->error() == QNetworkReply::NoError);
        reply->deleteLater();
        return success;
    }

    bool downloadFile(const QString& urlStr, const QString& savePath) {
        QUrl url(urlStr);
        QNetworkRequest request(url);

        QNetworkReply* reply = manager_->get(request);
        QEventLoop loop;
        connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
        loop.exec();

        if (reply->error() != QNetworkReply::NoError) {
            if (logCallback_) logCallback_("Download failed: " + reply->errorString().toStdString());
            reply->deleteLater();
            return false;
        }

        QFile file(savePath);
        if (!file.open(QIODevice::WriteOnly)) {
            qWarning() << "Could not open file for writing:" << savePath;
            reply->deleteLater();
            return false;
        }

        file.write(reply->readAll());
        file.close();
        reply->deleteLater();
        return true;
    }

    bool uploadResult(const QString& jobId, const QString& filePath) {
        QUrl url(baseUrl_ + "/api/jobs/" + jobId + "/complete");
        QNetworkRequest request(url);
        request.setRawHeader("Authorization", "Bearer " + oauthToken_.toUtf8());

        QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);

        QHttpPart filePart;
        filePart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"file\"; filename=\"result.png\""));
        QFile *file = new QFile(filePath);
        file->open(QIODevice::ReadOnly);
        filePart.setBodyDevice(file);
        file->setParent(multiPart);

        multiPart->append(filePart);

        QNetworkReply* reply = manager_->post(request, multiPart);
        multiPart->setParent(reply); 

        QEventLoop loop;
        connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
        loop.exec();

        bool success = (reply->error() == QNetworkReply::NoError);
        if (!success) {
             QString err = "Upload failed: " + reply->errorString();
             qWarning() << err;
             if (logCallback_) logCallback_(err.toStdString());
        }
        reply->deleteLater();
        return success;
    }
};

JobManager::JobManager(const std::string& nodeId, const std::string& oauthToken)
    : pImpl(new Impl(nodeId, oauthToken)) {}

JobManager::~JobManager() {
    delete pImpl;
}

common::Status JobManager::start() {
    pImpl->start();
    return common::Status::OK;
}

void JobManager::stop() {
    pImpl->stop();
}

void JobManager::setInterval(uint32_t seconds) {
    pImpl->setInterval(seconds);
}

void JobManager::setLogCallback(std::function<void(const std::string&)> callback) {
    pImpl->setLogCallback(callback);
}

} // namespace job_runner
} // namespace providers_agent

#include "job_manager.moc"
