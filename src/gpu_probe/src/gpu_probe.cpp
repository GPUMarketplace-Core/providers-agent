#include "gpu_probe/gpu_probe.h"

namespace providers_agent {
namespace gpu_probe {

// TODO: Implement using NVIDIA NVML
// - nvmlInit()
// - nvmlDeviceGetCount()
// - nvmlDeviceGetHandleByIndex()
// - nvmlDeviceGetName()
// - nvmlDeviceGetMemoryInfo()

GPUProbe::GPUProbe() : pImpl(nullptr) {
}

GPUProbe::~GPUProbe() {
    // TODO: nvmlShutdown()
}

common::Status GPUProbe::initialize() {
    // TODO: Initialize NVML
    return common::Status::OK;
}

common::Status GPUProbe::getGPUInfo(std::vector<common::GPUInfo>& gpus) {
    // TODO: Query GPU information
    return common::Status::ERROR_NOT_INITIALIZED;
}

nlohmann::json GPUProbe::toJson() const {
    // TODO: Export as JSON
    return nlohmann::json{};
}

common::Status GPUProbe::logToFile(const std::string& filepath) {
    // TODO: Write to log file
    return common::Status::ERROR_NOT_INITIALIZED;
}

} // namespace gpu_probe
} // namespace providers_agent
