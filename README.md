# Providers Agent - Desktop Client

Windows desktop client for managing Blender rendering tasks with hardware monitoring and heartbeat capabilities.

## Project Structure

```
providers-agent/
├── src/
│   ├── common/           # Shared utilities and interfaces
│   ├── system_probe/     # CPU and RAM monitoring (WinAPI)
│   ├── gpu_probe/        # GPU monitoring (NVML)
│   ├── heartbeat/        # HTTP heartbeat client
│   ├── credential_manager/  # Secure credential storage (DPAPI)
│   ├── storage/          # JSON file persistence
│   ├── gui/              # Win32/Qt main UI
│   ├── tray_agent/       # System tray agent
│   └── app/              # Main application
├── tests/                # Unit and integration tests
├── third_party/          # External dependencies
├── installer/            # MSI/Inno Setup configuration
└── docs/                 # Documentation
```

## Features

### System Tray Agent (Windows)
The application includes a Windows system tray icon that provides:
- **Visual Status Indicators**: 
  - 🔴 Red icon for Offline status
  - 🟢 Green icon for Online status
  - 🟡 Yellow icon for Busy status
- **Interactive Controls**:
  - Right-click menu for status changes and exit
  - Double-click to toggle between Online/Offline
  - Hover tooltip showing current status
- **Notifications**: Balloon notifications for status changes and events

See [Tray Agent Documentation](docs/tray_agent_usage.md) for detailed usage.

### Other Features
- Real-time CPU and RAM monitoring
- GPU statistics via NVIDIA NVML
- Secure credential storage using Windows DPAPI
- JSON-based persistent storage
- Heartbeat client for server communication

## Team Tasks

1. **System Probe** - CPU/RAM monitoring using WinAPI
2. **GUI Application** - Win32/Qt UI for displaying system info
3. **GPU Probe** - NVIDIA NVML integration for GPU stats
4. **Installer** - MSI/Inno Setup package
5. **Heartbeat Client** - HTTP POST every 10s
6. **Credential Manager** - Windows Credential Manager/DPAPI
7. **Storage Module** - JSON file I/O using nlohmann::json
8. **Tray Agent** - System tray status indicator with Online/Offline toggle
9. **Testing & Integration** - VM testing and demo

## Build Instructions

### Prerequisites
- Windows 10/11
- CMake 3.20+
- Visual Studio 2019/2022 with C++ tools
- NVIDIA CUDA Toolkit (for NVML)
- Qt 6.x (optional, for Qt GUI variant)

### Build Steps
```bash
mkdir build
cd build
cmake .. -G "Visual Studio 17 2022" -A x64
cmake --build . --config Release
```

## Development Workflow

1. Create feature branch: `git checkout -b feature/module-name`
2. Implement module in respective `src/` directory
3. Add tests in `tests/` directory
4. Update module CMakeLists.txt
5. Create PR for review

## Branch Naming Convention

- `feature/system-probe` - System monitoring implementation
- `feature/gpu-probe` - GPU monitoring implementation
- `feature/heartbeat` - Heartbeat client implementation
- `feature/credential-manager` - Credential storage implementation
- `feature/storage` - File I/O implementation
- `feature/gui` - UI implementation
- `feature/tray-agent` - Tray agent implementation
- `feature/installer` - Installer configuration
- `feature/testing` - Testing and integration

## License

[Add your license here]
