#include "system_probe/system_probe.h"

namespace providers_agent {
namespace system_probe {

// TODO: Implement using WinAPI
// - GetSystemInfo for CPU info
// - GlobalMemoryStatusEx for memory info

SystemProbe::SystemProbe() : pImpl(nullptr) {
}

SystemProbe::~SystemProbe() {
}

common::Status SystemProbe::initialize() {
    // TODO: Initialize WinAPI resources
    return common::Status::OK;
}

common::Status SystemProbe::getSystemInfo(common::SystemInfo& info) {
    // TODO: Implement CPU and RAM reading
    return common::Status::ERROR_NOT_INITIALIZED;
}

nlohmann::json SystemProbe::toJson() const {
    // TODO: Export as JSON
    return nlohmann::json{};
}

} // namespace system_probe
} // namespace providers_agent
