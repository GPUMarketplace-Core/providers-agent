# Architecture Overview

## Module Responsibilities

### 1. System Probe
- **Technology**: WinAPI (GetSystemInfo, GlobalMemoryStatusEx)
- **Responsibilities**:
  - Read CPU information (cores, frequency)
  - Read RAM usage (total, available, percentage)
  - Export data as JSON

### 2. GPU Probe
- **Technology**: NVIDIA NVML C++ API
- **Responsibilities**:
  - Detect NVIDIA GPUs
  - Read GPU name and VRAM
  - Log information to file

### 3. Heartbeat Client
- **Technology**: WinHTTP or libcurl
- **Responsibilities**:
  - Send HTTP POST requests every 10 seconds
  - Include timestamp in JSON payload
  - Handle network errors gracefully

### 4. Credential Manager
- **Technology**: Windows Credential Manager / DPAPI
- **Responsibilities**:
  - Store API tokens securely
  - Retrieve credentials when needed
  - Use OS-level encryption

### 5. Storage Module
- **Technology**: std::fstream + nlohmann::json
- **Responsibilities**:
  - Save hardware information to JSON files
  - Load hardware information from JSON files
  - Handle file I/O errors

### 6. GUI Application
- **Technology**: Win32 API or Qt
- **Responsibilities**:
  - Display main window
  - Button to launch System Probe
  - Text box to show results

### 7. Tray Agent
- **Technology**: Win32 Shell API
- **Responsibilities**:
  - Run in system tray
  - Show Online/Offline status icon
  - Display notifications

### 8. Installer
- **Technology**: WiX Toolset or Inno Setup
- **Responsibilities**:
  - Package application as MSI or EXE installer
  - Install to Program Files
  - Create shortcuts

## Data Flow

1. System/GPU probes collect hardware information
2. Information is serialized to JSON
3. Storage module persists JSON to file
4. Heartbeat client sends periodic updates to server
5. Credentials are stored securely using OS APIs
6. GUI displays information to user
7. Tray agent shows status in system tray

## Dependencies

- **nlohmann/json**: JSON serialization
- **NVIDIA NVML**: GPU information
- **WinAPI**: System info, GUI, tray, credentials
- **WinHTTP/libcurl**: HTTP communication
- **Qt** (optional): Cross-platform GUI
