#include "gpu_probe/gpu_probe.h"
#include <nvml.h>
#include <fstream>

namespace providers_agent {
namespace gpu_probe {

class GPUProbe::Impl {
public:
    bool initialized = false;
    std::vector<common::GPUInfo> cached_gpus;
};

GPUProbe::GPUProbe() : pImpl(new Impl()) {
}

GPUProbe::~GPUProbe() {
    if (pImpl->initialized) {
        nvmlShutdown();
    }
    delete pImpl;
}

common::Status GPUProbe::initialize() {
    nvmlReturn_t result = nvmlInit();
    if (result != NVML_SUCCESS) {
        return common::Status::ERROR_GENERAL;
    }
    pImpl->initialized = true;
    return common::Status::OK;
}

common::Status GPUProbe::getGPUInfo(std::vector<common::GPUInfo>& gpus) {
    if (!pImpl->initialized) {
        return common::Status::ERROR_NOT_INITIALIZED;
    }

    unsigned int device_count;
    nvmlReturn_t result = nvmlDeviceGetCount(&device_count);
    if (result != NVML_SUCCESS) {
        return common::Status::ERROR_GENERAL;
    }

    gpus.clear();
    for (unsigned int i = 0; i < device_count; i++) {
        nvmlDevice_t device;
        result = nvmlDeviceGetHandleByIndex(i, &device);
        if (result != NVML_SUCCESS) continue;

        common::GPUInfo gpu_info;
        
        // Get GPU name
        char name[NVML_DEVICE_NAME_BUFFER_SIZE];
        result = nvmlDeviceGetName(device, name, NVML_DEVICE_NAME_BUFFER_SIZE);
        if (result == NVML_SUCCESS) {
            gpu_info.name = std::string(name);
        }

        // Get memory info
        nvmlMemory_t memory;
        result = nvmlDeviceGetMemoryInfo(device, &memory);
        if (result == NVML_SUCCESS) {
            gpu_info.vram_total_bytes = memory.total;
            gpu_info.vram_used_bytes = memory.used;
        }

        // Get temperature
        unsigned int temp;
        result = nvmlDeviceGetTemperature(device, NVML_TEMPERATURE_GPU, &temp);
        if (result == NVML_SUCCESS) {
            gpu_info.temperature_celsius = temp;
        }

        // Get utilization
        nvmlUtilization_t utilization;
        result = nvmlDeviceGetUtilizationRates(device, &utilization);
        if (result == NVML_SUCCESS) {
            gpu_info.utilization_percent = utilization.gpu;
        }

        gpus.push_back(gpu_info);
    }

    pImpl->cached_gpus = gpus;
    return common::Status::OK;
}

nlohmann::json GPUProbe::toJson() const {
    nlohmann::json j;
    j["gpus"] = nlohmann::json::array();
    
    for (const auto& gpu : pImpl->cached_gpus) {
        nlohmann::json gpu_json;
        gpu_json["name"] = gpu.name;
        gpu_json["vram_total_bytes"] = gpu.vram_total_bytes;
        gpu_json["vram_used_bytes"] = gpu.vram_used_bytes;
        gpu_json["temperature_celsius"] = gpu.temperature_celsius;
        gpu_json["utilization_percent"] = gpu.utilization_percent;
        j["gpus"].push_back(gpu_json);
    }
    
    return j;
}

common::Status GPUProbe::logToFile(const std::string& filepath) {
    std::vector<common::GPUInfo> gpus;
    common::Status status = getGPUInfo(gpus);
    if (status != common::Status::OK) {
        return status;
    }

    std::ofstream file(filepath);
    if (!file.is_open()) {
        return common::Status::ERROR_FILE_IO;
    }

    nlohmann::json log_data = toJson();
    file << log_data.dump(4);
    file.close();

    return common::Status::OK;
}

} // namespace gpu_probe
} // namespace providers_agent
