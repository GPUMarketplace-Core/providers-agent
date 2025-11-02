#pragma once

#include "common/types.h"
#include <nlohmann/json.hpp>
#include <vector>

namespace providers_agent {
namespace gpu_probe {

class GPUProbe {
public:
    GPUProbe();
    ~GPUProbe();

    // Initialize NVML
    common::Status initialize();

    // Get GPU information
    common::Status getGPUInfo(std::vector<common::GPUInfo>& gpus);

    // Export GPU info as JSON
    nlohmann::json toJson() const;

    // Log to file
    common::Status logToFile(const std::string& filepath);

private:
    // TODO: Implement using NVIDIA NVML C++ API
    class Impl;
    Impl* pImpl;
};

} // namespace gpu_probe
} // namespace providers_agent
