#pragma once

#include <string>
#include <vector>
#include "common/types.h"

namespace providers_agent {
namespace gpu_probe {

class GPUProbe {
public:
    GPUProbe();
    ~GPUProbe();

    common::Status initialize();
    common::Status getGPUInfo(std::vector<common::GPUInfo>& gpus);
    common::Status logToFile(const std::string& path);
};

} // namespace gpu_probe
} // namespace providers_agent
