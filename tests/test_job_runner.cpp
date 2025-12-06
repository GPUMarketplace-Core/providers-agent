#include <iostream>
#include <QCoreApplication>
#include <QDebug>
#include <QTimer>
#include "job_runner/job_runner.h"

using namespace providers_agent;

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    std::cout << "=== Job Runner Test ===" << std::endl;

    job_runner::JobRunner runner;

    // 1. Check if Blender executable was found
    // The JobRunner constructor attempts to find it.
    // We can check if startBlender fails immediately to verify.
    
    std::cout << "Attempting to start Blender version check..." << std::endl;

    // 2. Try to run 'blender --version'
    // Note: --version causes Blender to print version and exit.
    // -b (background) might not be strictly necessary for --version but good practice.
    QStringList args;
    args << "--version"; 

    bool started = runner.startBlender(args);

    if (!started) {
        std::cerr << "✗ Failed to start Blender process." << std::endl;
        std::cerr << "  Make sure the 'blender' directory is copied to the executable directory." << std::endl;
        return 1;
    }

    std::cout << "✓ Blender process started." << std::endl;
    std::cout << "Waiting for process to finish..." << std::endl;

    // 3. Wait for finish
    bool finished = runner.waitForFinished(10000); // Wait up to 10 seconds

    if (!finished) {
        std::cerr << "✗ Process timed out or did not finish." << std::endl;
        return 1;
    }

    // 4. Check output
    QByteArray output = runner.readAllStandardOutput();
    QByteArray error = runner.readAllStandardError();

    std::cout << "Process finished." << std::endl;
    std::cout << "--- Standard Output ---" << std::endl;
    std::cout << output.toStdString() << std::endl;
    std::cout << "-----------------------" << std::endl;
    
    if (!error.isEmpty()) {
        std::cout << "--- Standard Error ---" << std::endl;
        std::cout << error.toStdString() << std::endl;
        std::cout << "----------------------" << std::endl;
    }

    if (output.contains("Blender")) {
        std::cout << "✓ Verified 'Blender' string in output." << std::endl;
        std::cout << "=== Test Passed ===" << std::endl;
        return 0;
    } else {
        std::cout << "✗ Could not verify 'Blender' in output." << std::endl;
        std::cout << "=== Test Failed ===" << std::endl;
        return 1;
    }
}

