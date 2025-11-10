#include <iostream>
#include "system_probe/system_probe.h"
#include "gpu_probe/gpu_probe.h"
#include "heartbeat/heartbeat_client.h"
#include "credential_manager/credential_manager.h"
#include "storage/json_storage.h"

// Include GUI for GUI mode
#include "gui/main_window.h"
// #include "tray_agent/tray_agent.h"

using namespace providers_agent;

// Console mode: Demonstrate system probe with formatted JSON output
int runConsoleDemo() {
    std::cout << "=================================================\n";
    std::cout << "     PROVIDERS AGENT - SYSTEM PROBE DEMO\n";
    std::cout << "=================================================\n\n";

    // Initialize system probe
    system_probe::SystemProbe probe;

    std::cout << "Initializing System Probe..." << std::endl;
    common::Status status = probe.initialize();

    if (status != common::Status::OK) {
        std::cerr << "ERROR: Failed to initialize System Probe" << std::endl;
        return 1;
    }
    std::cout << "System Probe initialized successfully.\n\n";

    // Get system information
    std::cout << "Reading CPU and RAM information..." << std::endl;
    common::SystemInfo sysInfo;
    status = probe.getSystemInfo(sysInfo);

    if (status != common::Status::OK) {
        std::cerr << "ERROR: Failed to get system information" << std::endl;
        return 1;
    }
    std::cout << "System information retrieved successfully.\n\n";

    // Export to JSON
    nlohmann::json jsonOutput = probe.toJson();

    std::cout << "=================================================\n";
    std::cout << "           SYSTEM INFORMATION (JSON)\n";
    std::cout << "=================================================\n";
    std::cout << jsonOutput.dump(4) << std::endl;  // Pretty-print with 4-space indent
    std::cout << "=================================================\n\n";

    return 0;
}

// GUI mode: Launch the main window
int runGuiMode() {
    gui::MainWindow window;

    common::Status status = window.initialize();
    if (status != common::Status::OK) {
        std::cerr << "ERROR: Failed to initialize GUI" << std::endl;
        return 1;
    }

    window.show();
    return window.exec();
}

int main(int argc, char* argv[]) {
    std::cout << "Providers Agent Starting..." << std::endl;

    // Check command-line arguments
    bool consoleMode = false;
    bool guiMode = false;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--console" || arg == "-c") {
            consoleMode = true;
        } else if (arg == "--gui" || arg == "-g") {
            guiMode = true;
        } else if (arg == "--help" || arg == "-h") {
            std::cout << "\nUsage: " << argv[0] << " [options]\n";
            std::cout << "Options:\n";
            std::cout << "  --console, -c    Run in console mode (demo system probe)\n";
            std::cout << "  --gui, -g        Run in GUI mode (Win32 window)\n";
            std::cout << "  --help, -h       Show this help message\n\n";
            std::cout << "Default: GUI mode\n";
            return 0;
        }
    }

    // Default to GUI mode if no option specified
    if (!consoleMode && !guiMode) {
        guiMode = true;
    }

    if (consoleMode) {
        return runConsoleDemo();
    } else {
        return runGuiMode();
    }
}
