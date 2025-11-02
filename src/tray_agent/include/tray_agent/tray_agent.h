#pragma once

#include "common/types.h"

namespace providers_agent {
namespace tray_agent {

enum class AgentStatus {
    ONLINE,
    OFFLINE,
    BUSY
};

class TrayAgent {
public:
    TrayAgent();
    ~TrayAgent();

    // Initialize tray icon
    common::Status initialize();

    // Set agent status (updates icon)
    void setStatus(AgentStatus status);

    // Show notification
    void showNotification(const std::string& title, const std::string& message);

    // Run tray agent
    int exec();

private:
    // TODO: Implement Windows system tray
    // - Shell_NotifyIcon for tray icon
    // - Switch between Online/Offline icons
    class Impl;
    Impl* pImpl;
};

} // namespace tray_agent
} // namespace providers_agent
