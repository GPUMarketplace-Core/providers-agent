#include <iostream>
#include "system_probe/system_probe.h"
#include "gpu_probe/gpu_probe.h"
#include "heartbeat/heartbeat_client.h"
#include "credential_manager/credential_manager.h"
#include "storage/json_storage.h"

// TODO: Integrate GUI and Tray Agent when implemented
// #include "gui/main_window.h"
// #include "tray_agent/tray_agent.h"

using namespace providers_agent;

int main(int argc, char* argv[]) {
    std::cout << "Providers Agent Starting..." << std::endl;

    // TODO: Initialize all modules
    // - System Probe
    // - GPU Probe
    // - Heartbeat Client
    // - Credential Manager
    // - Storage
    // - GUI (optional)
    // - Tray Agent

    std::cout << "Providers Agent Ready" << std::endl;

    // TODO: Run main application loop or GUI

    return 0;
}
