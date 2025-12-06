#include "gpu_probe/gpu_probe.h"
#include <iostream>
#include <fstream>
#include <nlohmann/json.hpp>
#include <set>

#ifdef _WIN32
    #include <windows.h>
#endif

namespace providers_agent {
namespace gpu_probe {

GPUProbe::GPUProbe() {}
GPUProbe::~GPUProbe() {}

common::Status GPUProbe::initialize() {
    return common::Status::OK;
}

common::Status GPUProbe::getGPUInfo(std::vector<common::GPUInfo>& gpus) {
    gpus.clear();

#ifdef _WIN32
    DISPLAY_DEVICEA dd;
    dd.cb = sizeof(DISPLAY_DEVICEA);
    int deviceIndex = 0;
    std::set<std::string> seenGPUs;

    while (EnumDisplayDevicesA(NULL, deviceIndex, &dd, 0)) {
        // Filter to physical adapters (usually attached to desktop or primary)
        // Note: This might miss headless GPUs, but it's a good start for standard Windows detection
        if ((dd.StateFlags & DISPLAY_DEVICE_PRIMARY_DEVICE) || 
            (dd.StateFlags & DISPLAY_DEVICE_ATTACHED_TO_DESKTOP) ||
            !(dd.StateFlags & DISPLAY_DEVICE_MIRRORING_DRIVER)) {
            
            std::string gpuName = dd.DeviceString;
            
            // Basic deduplication and cleanup
            if (!gpuName.empty() && seenGPUs.find(gpuName) == seenGPUs.end()) {
                common::GPUInfo gpu;
                gpu.name = gpuName;
                // VRAM/Temp/Util require more complex APIs (NVML/DXGI)
                // Setting simulated defaults or 0
                gpu.vram_total_bytes = 0; 
                gpu.vram_used_bytes = 0;
                gpu.temperature_celsius = 0;
                gpu.utilization_percent = 0;
                
                gpus.push_back(gpu);
                seenGPUs.insert(gpuName);
            }
        }
        deviceIndex++;
    }
    
    if (gpus.empty()) {
        // Fallback if EnumDisplayDevices found nothing useful (e.g. headless)
        // Could try WMI or registry here
    }
#else
    // Unix stub
#endif

    return common::Status::OK;
}

common::Status GPUProbe::logToFile(const std::string& path) {
    std::cout << "GPUProbe: Logging to file " << path << " (Stub)" << std::endl;
    
    // Write a dummy JSON file
    nlohmann::json j;
    j["gpus"] = nlohmann::json::array();
    j["gpus"].push_back({
        {"name", "NVIDIA GeForce RTX 3090 (Simulated)"},
        {"vram_total_bytes", 24576000000ULL}
    });

    try {
        std::ofstream file(path);
        file << j.dump(4);
        return common::Status::OK;
    } catch (const std::exception& e) {
        std::cerr << "GPUProbe: Failed to write file: " << e.what() << std::endl;
        return common::Status::ERROR_FILE_IO;
    }
}

} // namespace gpu_probe
} // namespace providers_agent
