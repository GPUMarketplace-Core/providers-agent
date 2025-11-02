#pragma once

#include "common/types.h"
#include <string>

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

    // Update display with system info
    void updateSystemInfo(const std::string& json_text);

    // Run message loop
    int exec();

private:
    // TODO: Implement using Win32 API or Qt
    // - Create window
    // - Launch System Probe button
    // - Text box to display results
    class Impl;
    Impl* pImpl;
};

} // namespace gui
} // namespace providers_agent
