#include "job_runner/job_runner.h"
#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QDebug>

namespace providers_agent {
namespace job_runner {

JobRunner::JobRunner() : process_(std::make_unique<QProcess>()) {
    findBlenderExecutable();

    connect(process_.get(), &QProcess::readyReadStandardOutput, this, [this]() {
        QByteArray data = process_->readAllStandardOutput();
        emit readyReadStandardOutput(QString::fromUtf8(data));
    });

    connect(process_.get(), &QProcess::readyReadStandardError, this, [this]() {
        QByteArray data = process_->readAllStandardError();
        emit readyReadStandardError(QString::fromUtf8(data));
    });

    connect(process_.get(), QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &JobRunner::finished);
}

JobRunner::~JobRunner() {
    if (process_->state() == QProcess::Running) {
        process_->kill();
        process_->waitForFinished();
    }
}

void JobRunner::findBlenderExecutable() {
    // The blender directory is expected to be copied to the bin directory
    // Structure: bin/blender/blender-5.0.0-windows-x64/blender.exe
    
    QDir appDir = QDir(QCoreApplication::applicationDirPath());
    
    // Adjust this path matching the copy structure in CMake
    // We will copy 'src/blender' content to 'blender' folder in bin
    QString potentialPath = appDir.filePath("blender/blender-5.0.0-windows-x64/blender.exe");

    if (QFileInfo::exists(potentialPath)) {
        blenderExecutablePath_ = potentialPath;
        qDebug() << "Found Blender executable at:" << blenderExecutablePath_;
    } else {
        qWarning() << "Blender executable not found at expected path:" << potentialPath;
        // Fallback or searching logic could go here
    }
}

bool JobRunner::startBlender(const QStringList& args) {
    if (blenderExecutablePath_.isEmpty()) {
        qCritical() << "Cannot start Blender: Executable path is empty.";
        return false;
    }

    if (process_->state() != QProcess::NotRunning) {
        qWarning() << "Blender process is already running.";
        return false;
    }

    process_->setProgram(blenderExecutablePath_);
    process_->setArguments(args);
    
    process_->start();
    
    if (!process_->waitForStarted()) {
        qCritical() << "Failed to start Blender process:" << process_->errorString();
        return false;
    }

    return true;
}

bool JobRunner::isRunning() const {
    return process_->state() != QProcess::NotRunning;
}

bool JobRunner::waitForFinished(int msecs) {
    return process_->waitForFinished(msecs);
}

QByteArray JobRunner::readAllStandardOutput() {
    return process_->readAllStandardOutput();
}

QByteArray JobRunner::readAllStandardError() {
    return process_->readAllStandardError();
}

} // namespace job_runner
} // namespace providers_agent
