#include "system_probe/system_probe.h"

#ifdef _WIN32
#include <windows.h>
#endif

namespace providers_agent {
namespace system_probe {

class SystemProbe::Impl {
public:
    common::SystemInfo cached_info;
    bool initialized = false;

#ifdef _WIN32
    // Get CPU information using WinAPI
    common::Status getCPUInfo(common::CPUInfo& cpu) {
        SYSTEM_INFO sysInfo;
        GetSystemInfo(&sysInfo);

        cpu.cores = sysInfo.dwNumberOfProcessors;
        cpu.logical_processors = sysInfo.dwNumberOfProcessors;

        // Processor architecture vendor detection
        switch (sysInfo.wProcessorArchitecture) {
            case PROCESSOR_ARCHITECTURE_AMD64:
                cpu.vendor = "x64 (AMD or Intel)";
                break;
            case PROCESSOR_ARCHITECTURE_ARM:
                cpu.vendor = "ARM";
                break;
            case PROCESSOR_ARCHITECTURE_ARM64:
                cpu.vendor = "ARM64";
                break;
            case PROCESSOR_ARCHITECTURE_INTEL:
                cpu.vendor = "x86 (Intel)";
                break;
            case PROCESSOR_ARCHITECTURE_IA64:
                cpu.vendor = "Intel Itanium";
                break;
            default:
                cpu.vendor = "Unknown";
                break;
        }

        // Note: GetSystemInfo doesn't provide frequency directly
        // In production, you'd read from registry or use WMI
        cpu.frequency_mhz = 0; // Placeholder

        return common::Status::OK;
    }

    // Get memory information using WinAPI
    common::Status getMemoryInfo(common::MemoryInfo& mem) {
        MEMORYSTATUSEX memStatus;
        memStatus.dwLength = sizeof(memStatus);

        if (!GlobalMemoryStatusEx(&memStatus)) {
            return common::Status::ERROR_GENERAL;
        }

        mem.total_bytes = memStatus.ullTotalPhys;
        mem.available_bytes = memStatus.ullAvailPhys;
        mem.usage_percent = memStatus.dwMemoryLoad;

        return common::Status::OK;
    }
#else
    // Placeholder for non-Windows platforms
    common::Status getCPUInfo(common::CPUInfo& cpu) {
        cpu.vendor = "Not supported on this platform";
        cpu.cores = 0;
        cpu.logical_processors = 0;
        cpu.frequency_mhz = 0;
        return common::Status::ERROR_GENERAL;
    }

    common::Status getMemoryInfo(common::MemoryInfo& mem) {
        mem.total_bytes = 0;
        mem.available_bytes = 0;
        mem.usage_percent = 0;
        return common::Status::ERROR_GENERAL;
    }
#endif
};

SystemProbe::SystemProbe() : pImpl(new Impl()) {
}

SystemProbe::~SystemProbe() {
    delete pImpl;
}

common::Status SystemProbe::initialize() {
    if (!pImpl) {
        return common::Status::ERROR_NOT_INITIALIZED;
    }

    pImpl->initialized = true;
    return common::Status::OK;
}

common::Status SystemProbe::getSystemInfo(common::SystemInfo& info) {
    if (!pImpl || !pImpl->initialized) {
        return common::Status::ERROR_NOT_INITIALIZED;
    }

    // Get CPU information
    common::Status status = pImpl->getCPUInfo(info.cpu);
    if (status != common::Status::OK) {
        return status;
    }

    // Get memory information
    status = pImpl->getMemoryInfo(info.memory);
    if (status != common::Status::OK) {
        return status;
    }

    // Cache the information
    pImpl->cached_info = info;

    return common::Status::OK;
}

nlohmann::json SystemProbe::toJson() const {
    if (!pImpl || !pImpl->initialized) {
        return nlohmann::json{
            {"error", "System probe not initialized"}
        };
    }

    const auto& info = pImpl->cached_info;

    nlohmann::json j;

    // CPU information
    j["cpu"] = {
        {"vendor", info.cpu.vendor},
        {"cores", info.cpu.cores},
        {"logical_processors", info.cpu.logical_processors},
        {"frequency_mhz", info.cpu.frequency_mhz}
    };

    // Memory information
    j["memory"] = {
        {"total_bytes", info.memory.total_bytes},
        {"available_bytes", info.memory.available_bytes},
        {"usage_percent", info.memory.usage_percent},
        {"total_gb", static_cast<double>(info.memory.total_bytes) / (1024 * 1024 * 1024)},
        {"available_gb", static_cast<double>(info.memory.available_bytes) / (1024 * 1024 * 1024)}
    };

    return j;
}

} // namespace system_probe
} // namespace providers_agent
