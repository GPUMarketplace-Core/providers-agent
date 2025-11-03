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

    // Test GPU Probe
    gpu_probe::GPUProbe gpu_probe;
    auto status = gpu_probe.initialize();
    if (status == common::Status::OK) {
        std::cout << "GPU Probe initialized successfully" << std::endl;
        
        // Log GPU info to file
        status = gpu_probe.logToFile("gpu_info.json");
        if (status == common::Status::OK) {
            std::cout << "GPU info logged to gpu_info.json" << std::endl;
        } else {
            std::cout << "Failed to log GPU info" << std::endl;
        }
    } else {
        std::cout << "Failed to initialize GPU Probe" << std::endl;
    }

    std::cout << "Providers Agent Ready" << std::endl;
    return 0;
}
