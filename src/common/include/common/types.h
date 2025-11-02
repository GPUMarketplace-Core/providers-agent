#pragma once

#include <string>
#include <cstdint>

namespace providers_agent {
namespace common {

// Common status codes
enum class Status {
    OK = 0,
    ERROR_GENERAL = 1,
    ERROR_NOT_INITIALIZED = 2,
    ERROR_INVALID_PARAMETER = 3,
    ERROR_FILE_IO = 4,
    ERROR_NETWORK = 5
};

// Hardware information structures
struct CPUInfo {
    std::string vendor;
    uint32_t cores;
    uint32_t logical_processors;
    uint64_t frequency_mhz;
};

struct MemoryInfo {
    uint64_t total_bytes;
    uint64_t available_bytes;
    uint32_t usage_percent;
};

struct GPUInfo {
    std::string name;
    uint64_t vram_total_bytes;
    uint64_t vram_used_bytes;
    uint32_t temperature_celsius;
    uint32_t utilization_percent;
};

struct SystemInfo {
    CPUInfo cpu;
    MemoryInfo memory;
};

} // namespace common
} // namespace providers_agent
