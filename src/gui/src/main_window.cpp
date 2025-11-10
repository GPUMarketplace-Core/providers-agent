#include "gui/main_window.h"
#include "system_probe/system_probe.h"

#ifdef _WIN32
#include <windows.h>
#include <string>
#endif

namespace providers_agent {
namespace gui {

#ifdef _WIN32
// Window procedure forward declaration
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

// Control IDs
#define ID_BUTTON_PROBE 1001
#define ID_TEXTBOX_OUTPUT 1002

class MainWindow::Impl {
public:
    HWND hwnd = nullptr;           // Main window handle
    HWND hButton = nullptr;        // Button handle
    HWND hTextBox = nullptr;       // Text box handle
    HINSTANCE hInstance = nullptr;
    system_probe::SystemProbe* probe = nullptr;

    // Store instance pointer for window procedure callback
    static Impl* instance;

    Impl() {
        instance = this;
        probe = new system_probe::SystemProbe();
    }

    ~Impl() {
        delete probe;
        if (hwnd) {
            DestroyWindow(hwnd);
        }
    }

    // Handle button click
    void onProbeButtonClick() {
        if (!probe) return;

        // Initialize probe if not already done
        common::Status status = probe->initialize();
        if (status != common::Status::OK) {
            SetWindowTextA(hTextBox, "ERROR: Failed to initialize System Probe");
            return;
        }

        // Get system information
        common::SystemInfo sysInfo;
        status = probe->getSystemInfo(sysInfo);
        if (status != common::Status::OK) {
            SetWindowTextA(hTextBox, "ERROR: Failed to get system information");
            return;
        }

        // Convert to JSON and display
        nlohmann::json jsonOutput = probe->toJson();
        std::string prettyJson = jsonOutput.dump(4);
        SetWindowTextA(hTextBox, prettyJson.c_str());
    }

    // Window procedure
    static LRESULT CALLBACK WindowProcStatic(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
        switch (uMsg) {
            case WM_CREATE:
                return 0;

            case WM_COMMAND:
                if (LOWORD(wParam) == ID_BUTTON_PROBE) {
                    if (instance) {
                        instance->onProbeButtonClick();
                    }
                }
                return 0;

            case WM_DESTROY:
                PostQuitMessage(0);
                return 0;

            case WM_SIZE: {
                // Resize controls when window is resized
                if (instance && instance->hButton && instance->hTextBox) {
                    RECT rect;
                    GetClientRect(hwnd, &rect);

                    int padding = 10;
                    int buttonHeight = 40;
                    int buttonWidth = 150;

                    // Position button at the top
                    SetWindowPos(instance->hButton, nullptr,
                                 padding, padding,
                                 buttonWidth, buttonHeight,
                                 SWP_NOZORDER);

                    // Position text box below button
                    int textBoxTop = padding * 2 + buttonHeight;
                    SetWindowPos(instance->hTextBox, nullptr,
                                 padding, textBoxTop,
                                 rect.right - padding * 2,
                                 rect.bottom - textBoxTop - padding,
                                 SWP_NOZORDER);
                }
                return 0;
            }

            default:
                return DefWindowProc(hwnd, uMsg, wParam, lParam);
        }
    }
};

// Initialize static member
MainWindow::Impl* MainWindow::Impl::instance = nullptr;

#else
// Non-Windows stub implementation
class MainWindow::Impl {
public:
    Impl() {}
    ~Impl() {}
};
#endif

MainWindow::MainWindow() : pImpl(new Impl()) {
}

MainWindow::~MainWindow() {
    delete pImpl;
}

common::Status MainWindow::initialize() {
#ifdef _WIN32
    if (!pImpl) {
        return common::Status::ERROR_NOT_INITIALIZED;
    }

    pImpl->hInstance = GetModuleHandle(nullptr);

    // Register window class
    const char* CLASS_NAME = "ProvidersAgentWindowClass";

    WNDCLASSA wc = {};
    wc.lpfnWndProc = Impl::WindowProcStatic;
    wc.hInstance = pImpl->hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

    if (!RegisterClassA(&wc)) {
        DWORD error = GetLastError();
        if (error != ERROR_CLASS_ALREADY_EXISTS) {
            return common::Status::ERROR_GENERAL;
        }
    }

    // Create window
    pImpl->hwnd = CreateWindowExA(
        0,                              // Optional window styles
        CLASS_NAME,                     // Window class
        "Providers Agent - System Monitor",  // Window title
        WS_OVERLAPPEDWINDOW,           // Window style
        CW_USEDEFAULT, CW_USEDEFAULT,  // Position
        800, 600,                      // Size
        nullptr,                       // Parent window
        nullptr,                       // Menu
        pImpl->hInstance,              // Instance handle
        nullptr                        // Additional data
    );

    if (!pImpl->hwnd) {
        return common::Status::ERROR_GENERAL;
    }

    // Create "Probe System" button
    pImpl->hButton = CreateWindowA(
        "BUTTON",                      // Predefined class
        "Probe System",                // Button text
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
        10, 10,                        // Position
        150, 40,                       // Size
        pImpl->hwnd,                   // Parent window
        (HMENU)ID_BUTTON_PROBE,       // Button ID
        pImpl->hInstance,
        nullptr
    );

    // Create multi-line text box for output
    pImpl->hTextBox = CreateWindowExA(
        WS_EX_CLIENTEDGE,              // Extended style
        "EDIT",                        // Predefined class
        "",                            // Initial text
        WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_LEFT | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY,
        10, 60,                        // Position
        780, 500,                      // Size
        pImpl->hwnd,                   // Parent window
        (HMENU)ID_TEXTBOX_OUTPUT,     // Control ID
        pImpl->hInstance,
        nullptr
    );

    // Set a monospace font for better JSON display
    HFONT hFont = CreateFontA(
        16,                           // Height
        0,                            // Width
        0,                            // Escapement
        0,                            // Orientation
        FW_NORMAL,                    // Weight
        FALSE,                        // Italic
        FALSE,                        // Underline
        FALSE,                        // Strikeout
        ANSI_CHARSET,                 // Character set
        OUT_DEFAULT_PRECIS,           // Output precision
        CLIP_DEFAULT_PRECIS,          // Clipping precision
        DEFAULT_QUALITY,              // Quality
        FIXED_PITCH | FF_MODERN,      // Pitch and family
        "Consolas"                    // Font name
    );

    if (hFont) {
        SendMessage(pImpl->hTextBox, WM_SETFONT, (WPARAM)hFont, TRUE);
    }

    // Set initial text
    SetWindowTextA(pImpl->hTextBox,
        "Click 'Probe System' to retrieve CPU and RAM information.\n\n"
        "The results will be displayed here in JSON format.");

    return common::Status::OK;
#else
    return common::Status::ERROR_GENERAL;
#endif
}

void MainWindow::show() {
#ifdef _WIN32
    if (pImpl && pImpl->hwnd) {
        ShowWindow(pImpl->hwnd, SW_SHOW);
        UpdateWindow(pImpl->hwnd);
    }
#endif
}

void MainWindow::updateSystemInfo(const std::string& json_text) {
#ifdef _WIN32
    if (pImpl && pImpl->hTextBox) {
        SetWindowTextA(pImpl->hTextBox, json_text.c_str());
    }
#endif
}

int MainWindow::exec() {
#ifdef _WIN32
    if (!pImpl || !pImpl->hwnd) {
        return -1;
    }

    // Run message loop
    MSG msg = {};
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int)msg.wParam;
#else
    return -1;
#endif
}

} // namespace gui
} // namespace providers_agent
