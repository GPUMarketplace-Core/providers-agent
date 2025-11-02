#include "gui/main_window.h"

namespace providers_agent {
namespace gui {

// TODO: Implement using Win32 or Qt
// - Create main window with button and text box
// - Button to launch System Probe
// - Display results in text box

MainWindow::MainWindow() : pImpl(nullptr) {
}

MainWindow::~MainWindow() {
}

common::Status MainWindow::initialize() {
    // TODO: Create window
    return common::Status::ERROR_NOT_INITIALIZED;
}

void MainWindow::show() {
    // TODO: Show window
}

void MainWindow::updateSystemInfo(const std::string& json_text) {
    // TODO: Update text box with system info
}

int MainWindow::exec() {
    // TODO: Run message loop
    return 0;
}

} // namespace gui
} // namespace providers_agent
