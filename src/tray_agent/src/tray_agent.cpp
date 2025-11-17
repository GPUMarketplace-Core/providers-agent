#include "tray_agent/tray_agent.h"

#ifdef _WIN32
#include <map>
#include <shellapi.h>
#include <string>
#include <windows.h>
#endif

namespace providers_agent {
namespace tray_agent {

#ifdef _WIN32

// Windows Message IDs
#define WM_TRAYICON (WM_USER + 1)
#define ID_TRAY_ICON 1001
#define ID_MENU_ONLINE 2001
#define ID_MENU_OFFLINE 2002
#define ID_MENU_EXIT 2003

// Implementation class for Windows
class TrayAgent::Impl {
public:
  HWND hwnd;
  NOTIFYICONDATAW nid;
  AgentStatus currentStatus;
  HICON iconOnline;
  HICON iconOffline;
  HICON iconBusy;
  bool initialized;
  bool running;

  Impl()
      : hwnd(nullptr), currentStatus(AgentStatus::OFFLINE), iconOnline(nullptr),
        iconOffline(nullptr), iconBusy(nullptr), initialized(false),
        running(false) {
    ZeroMemory(&nid, sizeof(NOTIFYICONDATAW));
  }

  ~Impl() { cleanup(); }

  void cleanup() {
    if (initialized && hwnd) {
      Shell_NotifyIconW(NIM_DELETE, &nid);
    }
    if (iconOnline)
      DestroyIcon(iconOnline);
    if (iconOffline)
      DestroyIcon(iconOffline);
    if (iconBusy)
      DestroyIcon(iconBusy);
    if (hwnd)
      DestroyWindow(hwnd);
    iconOnline = nullptr;
    iconOffline = nullptr;
    iconBusy = nullptr;
    hwnd = nullptr;
    initialized = false;
  }

  HICON createColoredIcon(COLORREF color) {
    // Create a simple colored icon (16x16)
    const int size = 16;
    HDC hdcScreen = GetDC(NULL);
    HDC hdcMem = CreateCompatibleDC(hdcScreen);
    HBITMAP hbmColor = CreateCompatibleBitmap(hdcScreen, size, size);
    HBITMAP hbmMask = CreateCompatibleBitmap(hdcScreen, size, size);

    // Draw colored circle
    HBITMAP hbmOld = (HBITMAP)SelectObject(hdcMem, hbmColor);
    HBRUSH hbrBg = CreateSolidBrush(RGB(255, 255, 255));
    RECT rect = {0, 0, size, size};
    FillRect(hdcMem, &rect, hbrBg);
    DeleteObject(hbrBg);

    HBRUSH hbrColor = CreateSolidBrush(color);
    HPEN hpenOld = (HPEN)SelectObject(hdcMem, GetStockObject(NULL_PEN));
    SelectObject(hdcMem, hbrColor);
    Ellipse(hdcMem, 2, 2, size - 2, size - 2);
    DeleteObject(hbrColor);
    SelectObject(hdcMem, hpenOld);

    // Create mask (all opaque)
    SelectObject(hdcMem, hbmMask);
    HBRUSH hbrBlack = CreateSolidBrush(RGB(0, 0, 0));
    FillRect(hdcMem, &rect, hbrBlack);
    DeleteObject(hbrBlack);

    SelectObject(hdcMem, hbmOld);
    DeleteDC(hdcMem);
    ReleaseDC(NULL, hdcScreen);

    ICONINFO ii;
    ii.fIcon = TRUE;
    ii.xHotspot = 0;
    ii.yHotspot = 0;
    ii.hbmMask = hbmMask;
    ii.hbmColor = hbmColor;

    HICON hIcon = CreateIconIndirect(&ii);

    DeleteObject(hbmColor);
    DeleteObject(hbmMask);

    return hIcon;
  }

  void createIcons() {
    // Create colored icons: Green for Online, Red for Offline, Yellow for Busy
    iconOnline = createColoredIcon(RGB(0, 255, 0));  // Green
    iconOffline = createColoredIcon(RGB(255, 0, 0)); // Red
    iconBusy = createColoredIcon(RGB(255, 255, 0));  // Yellow
  }

  HICON getIconForStatus(AgentStatus status) {
    switch (status) {
    case AgentStatus::ONLINE:
      return iconOnline;
    case AgentStatus::BUSY:
      return iconBusy;
    case AgentStatus::OFFLINE:
    default:
      return iconOffline;
    }
  }

  void updateTrayIcon() {
    if (!initialized)
      return;

    nid.hIcon = getIconForStatus(currentStatus);
    Shell_NotifyIconW(NIM_MODIFY, &nid);
  }

  void showContextMenu() {
    POINT pt;
    GetCursorPos(&pt);

    HMENU hMenu = CreatePopupMenu();
    AppendMenuW(hMenu,
                MF_STRING |
                    (currentStatus == AgentStatus::ONLINE ? MF_CHECKED : 0),
                ID_MENU_ONLINE, L"Set Online");
    AppendMenuW(hMenu,
                MF_STRING |
                    (currentStatus == AgentStatus::OFFLINE ? MF_CHECKED : 0),
                ID_MENU_OFFLINE, L"Set Offline");
    AppendMenuW(hMenu, MF_SEPARATOR, 0, NULL);
    AppendMenuW(hMenu, MF_STRING, ID_MENU_EXIT, L"Exit");

    // Required for popup menu to close properly when clicking outside
    SetForegroundWindow(hwnd);

    TrackPopupMenu(hMenu, TPM_RIGHTBUTTON, pt.x, pt.y, 0, hwnd, NULL);
    DestroyMenu(hMenu);

    // Required after TrackPopupMenu
    PostMessage(hwnd, WM_NULL, 0, 0);
  }

  void handleCommand(WPARAM wParam) {
    switch (LOWORD(wParam)) {
    case ID_MENU_ONLINE:
      currentStatus = AgentStatus::ONLINE;
      updateTrayIcon();
      break;
    case ID_MENU_OFFLINE:
      currentStatus = AgentStatus::OFFLINE;
      updateTrayIcon();
      break;
    case ID_MENU_EXIT:
      running = false;
      PostQuitMessage(0);
      break;
    }
  }

  static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam,
                                     LPARAM lParam) {
    TrayAgent::Impl *pThis = nullptr;

    if (uMsg == WM_CREATE) {
      CREATESTRUCT *pCreate = reinterpret_cast<CREATESTRUCT *>(lParam);
      pThis = reinterpret_cast<TrayAgent::Impl *>(pCreate->lpCreateParams);
      SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));
    } else {
      pThis = reinterpret_cast<TrayAgent::Impl *>(
          GetWindowLongPtr(hwnd, GWLP_USERDATA));
    }

    if (pThis) {
      switch (uMsg) {
      case WM_TRAYICON:
        if (lParam == WM_RBUTTONUP || lParam == WM_CONTEXTMENU) {
          pThis->showContextMenu();
        } else if (lParam == WM_LBUTTONDBLCLK) {
          // Toggle status on double-click
          if (pThis->currentStatus == AgentStatus::ONLINE) {
            pThis->currentStatus = AgentStatus::OFFLINE;
          } else {
            pThis->currentStatus = AgentStatus::ONLINE;
          }
          pThis->updateTrayIcon();
        }
        return 0;

      case WM_COMMAND:
        pThis->handleCommand(wParam);
        return 0;

      case WM_DESTROY:
        pThis->running = false;
        PostQuitMessage(0);
        return 0;
      }
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
  }
};

#else
// Stub implementation for non-Windows platforms
class TrayAgent::Impl {
public:
  bool initialized = false;
  AgentStatus currentStatus = AgentStatus::OFFLINE;
};
#endif

// TrayAgent implementation
TrayAgent::TrayAgent() : pImpl(new Impl()) {}

TrayAgent::~TrayAgent() { delete pImpl; }

common::Status TrayAgent::initialize() {
#ifdef _WIN32
  if (pImpl->initialized) {
    return common::Status::OK;
  }

  // Register window class
  WNDCLASSEXW wc = {};
  wc.cbSize = sizeof(WNDCLASSEXW);
  wc.lpfnWndProc = Impl::WindowProc;
  wc.hInstance = GetModuleHandle(NULL);
  wc.lpszClassName = L"ProvidersAgentTrayClass";

  if (!RegisterClassExW(&wc)) {
    return common::Status::ERROR_GENERAL;
  }

  // Create hidden window
  pImpl->hwnd =
      CreateWindowExW(0, L"ProvidersAgentTrayClass", L"Providers Agent", 0, 0,
                      0, 0, 0, NULL, NULL, GetModuleHandle(NULL), pImpl);

  if (!pImpl->hwnd) {
    return common::Status::ERROR_GENERAL;
  }

  // Create icons
  pImpl->createIcons();

  // Setup tray icon
  pImpl->nid.cbSize = sizeof(NOTIFYICONDATAW);
  pImpl->nid.hWnd = pImpl->hwnd;
  pImpl->nid.uID = ID_TRAY_ICON;
  pImpl->nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
  pImpl->nid.uCallbackMessage = WM_TRAYICON;
  pImpl->nid.hIcon = pImpl->getIconForStatus(pImpl->currentStatus);
  wcscpy_s(pImpl->nid.szTip, L"Providers Agent - Offline");

  if (!Shell_NotifyIconW(NIM_ADD, &pImpl->nid)) {
    DestroyWindow(pImpl->hwnd);
    return common::Status::ERROR_GENERAL;
  }

  pImpl->initialized = true;
  pImpl->running = true;
  return common::Status::OK;
#else
  return common::Status::ERROR_NOT_INITIALIZED;
#endif
}

void TrayAgent::setStatus(AgentStatus status) {
#ifdef _WIN32
  if (!pImpl->initialized)
    return;

  pImpl->currentStatus = status;

  // Update tooltip text
  const wchar_t *statusText = L"Unknown";
  switch (status) {
  case AgentStatus::ONLINE:
    statusText = L"Online";
    break;
  case AgentStatus::OFFLINE:
    statusText = L"Offline";
    break;
  case AgentStatus::BUSY:
    statusText = L"Busy";
    break;
  }

  swprintf_s(pImpl->nid.szTip, L"Providers Agent - %s", statusText);
  pImpl->updateTrayIcon();
#endif
}

void TrayAgent::showNotification(const std::string &title,
                                 const std::string &message) {
#ifdef _WIN32
  if (!pImpl->initialized)
    return;

  // Convert strings to wide strings
  int titleLen = MultiByteToWideChar(CP_UTF8, 0, title.c_str(), -1, NULL, 0);
  int messageLen =
      MultiByteToWideChar(CP_UTF8, 0, message.c_str(), -1, NULL, 0);

  wchar_t wTitle[64] = {0};
  wchar_t wMessage[256] = {0};

  MultiByteToWideChar(CP_UTF8, 0, title.c_str(), -1, wTitle, min(titleLen, 64));
  MultiByteToWideChar(CP_UTF8, 0, message.c_str(), -1, wMessage,
                      min(messageLen, 256));

  pImpl->nid.uFlags |= NIF_INFO;
  wcscpy_s(pImpl->nid.szInfoTitle, wTitle);
  wcscpy_s(pImpl->nid.szInfo, wMessage);
  pImpl->nid.dwInfoFlags = NIIF_INFO;
  pImpl->nid.uTimeout = 3000; // 3 seconds (deprecated but still works)

  Shell_NotifyIconW(NIM_MODIFY, &pImpl->nid);

  // Reset flags
  pImpl->nid.uFlags &= ~NIF_INFO;
#endif
}

int TrayAgent::exec() {
#ifdef _WIN32
  if (!pImpl->initialized || !pImpl->running) {
    return -1;
  }

  // Message loop
  MSG msg;
  while (pImpl->running && GetMessage(&msg, NULL, 0, 0)) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }

  return 0;
#else
  return -1;
#endif
}

} // namespace tray_agent
} // namespace providers_agent
