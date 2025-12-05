#include "system_probe/system_probe.h"
#include <nlohmann/json.hpp>

#ifdef _WIN32
    #include <windows.h>
    #include <sysinfoapi.h>
#else
    // macOS/Linux
    #include <sys/sysctl.h>
    #include <sys/types.h>
    #include <mach/mach.h>
    #include <mach/mach_host.h>
    #include <unistd.h>
    #include <cstring>
#endif

namespace providers_agent {
namespace system_probe {

class SystemProbe::Impl {
public:
    common::SystemInfo cached_info;
    bool initialized;

    Impl() : initialized(false) {}

#ifdef _WIN32
    common::Status getSystemInfoWin32(common::SystemInfo& info) {
        // CPU Info
        SYSTEM_INFO sysInfo;
        GetSystemInfo(&sysInfo);
        
        info.cpu.cores = sysInfo.dwNumberOfProcessors;
        info.cpu.logical_processors = sysInfo.dwNumberOfProcessors;
        
        // Get CPU frequency from registry or WMI (simplified here)
        // For now, we'll use a default or query it
        info.cpu.frequency_mhz = 0; // TODO: Query actual frequency
        
        // Memory Info
        MEMORYSTATUSEX memStatus;
        memStatus.dwLength = sizeof(MEMORYSTATUSEX);
        GlobalMemoryStatusEx(&memStatus);
        
        info.memory.total_bytes = memStatus.ullTotalPhys;
        info.memory.available_bytes = memStatus.ullAvailPhys;
        info.memory.usage_percent = memStatus.dwMemoryLoad;
        
        return common::Status::OK;
    }
#else
    common::Status getSystemInfoUnix(common::SystemInfo& info) {
        // CPU Info - macOS/Linux
        int numCores = 0;
        size_t len = sizeof(numCores);
        
        // Get number of physical cores
        if (sysctlbyname("hw.physicalcpu", &numCores, &len, NULL, 0) == 0) {
            info.cpu.cores = numCores;
        } else {
            // Fallback to logical cores
            numCores = sysconf(_SC_NPROCESSORS_ONLN);
            info.cpu.cores = numCores;
        }
        
        // Get logical processors
        int logicalCores = 0;
        len = sizeof(logicalCores);
        if (sysctlbyname("hw.logicalcpu", &logicalCores, &len, NULL, 0) == 0) {
            info.cpu.logical_processors = logicalCores;
        } else {
            info.cpu.logical_processors = sysconf(_SC_NPROCESSORS_ONLN);
        }
        
        // Get CPU frequency (MHz)
        uint64_t freq = 0;
        len = sizeof(freq);
        if (sysctlbyname("hw.cpufrequency", &freq, &len, NULL, 0) == 0) {
            info.cpu.frequency_mhz = freq / 1000000; // Convert Hz to MHz
        } else {
            info.cpu.frequency_mhz = 0; // Unknown
        }
        
        // Get CPU vendor (simplified)
        char vendor[128] = {0};
        len = sizeof(vendor);
        if (sysctlbyname("machdep.cpu.vendor", vendor, &len, NULL, 0) == 0) {
            info.cpu.vendor = std::string(vendor);
        } else {
            info.cpu.vendor = "Unknown";
        }
        
        // Memory Info - macOS
        vm_size_t page_size;
        vm_statistics64_data_t vm_stat;
        mach_port_t mach_port = mach_host_self();
        mach_msg_type_number_t count = sizeof(vm_stat) / sizeof(natural_t);
        
        if (host_page_size(mach_port, &page_size) == KERN_SUCCESS &&
            host_statistics64(mach_port, HOST_VM_INFO, (host_info64_t)&vm_stat, &count) == KERN_SUCCESS) {
            
            uint64_t total_mem = 0;
            len = sizeof(total_mem);
            if (sysctlbyname("hw.memsize", &total_mem, &len, NULL, 0) == 0) {
                info.memory.total_bytes = total_mem;
            }
            
            uint64_t free_mem = (vm_stat.free_count + vm_stat.inactive_count) * page_size;
            info.memory.available_bytes = free_mem;
            
            if (info.memory.total_bytes > 0) {
                uint64_t used = info.memory.total_bytes - free_mem;
                info.memory.usage_percent = static_cast<uint32_t>(
                    (used * 100) / info.memory.total_bytes
                );
            } else {
                info.memory.usage_percent = 0;
            }
        } else {
            return common::Status::ERROR_GENERAL;
        }
        
        return common::Status::OK;
    }
#endif
};

SystemProbe::SystemProbe() : pImpl(new Impl()) {
}

SystemProbe::~SystemProbe() {
    delete pImpl;
}

common::Status SystemProbe::initialize() {
    pImpl->initialized = true;
    return common::Status::OK;
}

common::Status SystemProbe::getSystemInfo(common::SystemInfo& info) {
    if (!pImpl->initialized) {
        return common::Status::ERROR_NOT_INITIALIZED;
    }
    
#ifdef _WIN32
    return pImpl->getSystemInfoWin32(info);
#else
    return pImpl->getSystemInfoUnix(info);
#endif
}

nlohmann::json SystemProbe::toJson() const {
    if (!pImpl->initialized) {
        return nlohmann::json{};
    }
    
    common::SystemInfo info;
    // Create a non-const reference to call getSystemInfo
    SystemProbe* nonConstThis = const_cast<SystemProbe*>(this);
    if (nonConstThis->getSystemInfo(info) != common::Status::OK) {
        return nlohmann::json{};
    }
    
    return nlohmann::json{
        {"cpu", {
            {"vendor", info.cpu.vendor},
            {"cores", info.cpu.cores},
            {"logical_processors", info.cpu.logical_processors},
            {"frequency_mhz", info.cpu.frequency_mhz}
        }},
        {"memory", {
            {"total_bytes", info.memory.total_bytes},
            {"available_bytes", info.memory.available_bytes},
            {"usage_percent", info.memory.usage_percent}
        }}
    };
}

} // namespace system_probe
} // namespace providers_agent
