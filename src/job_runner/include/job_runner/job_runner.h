#pragma once

#include <string>
#include <vector>
#include <memory>
#include <QProcess>
#include <QObject>

namespace providers_agent {
namespace job_runner {

class JobRunner : public QObject {
    Q_OBJECT

public:
    JobRunner();
    ~JobRunner();

    /**
     * @brief Starts Blender with the given arguments.
     * @param args The arguments to pass to the Blender executable.
     * @return True if the process started successfully, false otherwise.
     */
    bool startBlender(const QStringList& args);

    /**
     * @brief Checks if the Blender process is currently running.
     * @return True if running, false otherwise.
     */
    bool isRunning() const;

    /**
     * @brief Waits for the process to finish.
     * @param msecs The timeout in milliseconds (-1 for no timeout).
     * @return True if finished, false on timeout or error.
     */
    bool waitForFinished(int msecs = -1);

    /**
     * @brief Gets the standard output from the process.
     */
    QByteArray readAllStandardOutput();

    /**
     * @brief Gets the standard error from the process.
     */
    QByteArray readAllStandardError();

signals:
    void readyReadStandardOutput(const QString& output);
    void readyReadStandardError(const QString& output);
    void finished(int exitCode, QProcess::ExitStatus exitStatus);

private:
    std::unique_ptr<QProcess> process_;
    QString blenderExecutablePath_;

    /**
     * @brief Locates the Blender executable in the bundled files.
     */
    void findBlenderExecutable();
};

} // namespace job_runner
} // namespace providers_agent
