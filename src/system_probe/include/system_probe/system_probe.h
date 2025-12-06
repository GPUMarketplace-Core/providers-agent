#pragma once

#include "common/types.h"
#include <nlohmann/json.hpp>

namespace providers_agent {
namespace system_probe {

class SystemProbe {
public:
    SystemProbe();
    ~SystemProbe();

    // Initialize the probe
    common::Status initialize();

    // Get current system information
    common::Status getSystemInfo(common::SystemInfo& info);

    // Export system info as JSON
    nlohmann::json toJson() const;

    // Get unique system identifier (MachineGuid/HWID)
    std::string getUniqueIdentifier();

private:
    // TODO: Implement using WinAPI (GetSystemInfo, GlobalMemoryStatusEx)
    class Impl;
    Impl* pImpl;
};

} // namespace system_probe
} // namespace providers_agent
