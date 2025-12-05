#pragma once

#include "common/types.h"
#include <string>
#include <functional>

// Forward declarations to avoid including Qt headers in header file
class QApplication;

namespace providers_agent {
namespace gui {

class MainWindow {
public:
    MainWindow();
    ~MainWindow();

    // Initialize window
    common::Status initialize();

    // Show window
    void show();

    // GPU Info structure for display
    struct GPUInfo {
        std::string name;
        std::string vram;
        std::string status;  // "InProgress", "Completed", "Error Occurred", "Idle"
        int progressPercent;
    };

    // Update display with system info
    void updateSystemInfo(const std::string& json_text);

    // Update heartbeat status
    void updateHeartbeatStatus(bool running, int count, const std::string& lastMessage = "", bool lastSuccess = true);

    // Update connection status
    void updateConnectionStatus(bool connected, const std::string& machineName = "", const std::string& agentVersion = "", int lastSyncMinutes = 0);

    // Update GPU list
    void updateGPUs(const std::vector<GPUInfo>& gpus);

    // Update system information display
    void updateSystemInfoDisplay(const std::string& cpuInfo, const std::string& memoryInfo);

    // Add log entry
    void addLog(const std::string& logEntry);

    // Update Go Online toggle
    void setGoOnline(bool online);

    // Run message loop
    int exec();

    // Callbacks for button clicks
    std::function<void()> onRefreshSystemInfo;
    std::function<void(bool)> onGoOnlineChanged;

private:
    class Impl;
    Impl* pImpl;
};

} // namespace gui
} // namespace providers_agent
