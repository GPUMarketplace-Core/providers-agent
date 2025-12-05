# Providers Agent GPU Probe Installer

## Prerequisites

1. **Inno Setup 6** - Download from https://jrsoftware.org/isinfo.php
2. **Visual Studio 2019/2022** with C++ tools
3. **CMake 3.20+**
4. **NVIDIA CUDA Toolkit** (for NVML)

## Building the Installer

### Option 1: Automated Build
```bash
cd installer
build_installer.bat
```

### Option 2: Manual Build
```bash
# 1. Build the application
mkdir build && cd build
cmake .. -G "Visual Studio 17 2022" -A x64
cmake --build . --config Release

# 2. Build installer
cmake --build . --target installer --config Release
```

### Option 3: Direct Inno Setup
```bash
# If you have the exe built already
"C:\Program Files\Inno Setup 6\iscc.exe" setup.iss
```

## Installer Features

- **Application**: Installs providers_agent.exe
- **Dependencies**: Includes required DLLs
- **Shortcuts**: Start menu + optional desktop icon
- **Directories**: Creates logs folder
- **Cleanup**: Removes log files on uninstall
- **Requirements**: Windows 10+ x64, Admin privileges

## Output

The installer will be created as:
`ProvidersAgent-GPUProbe-Setup.exe`

## Installation Path

Default: `C:\Program Files\ProvidersAgent\`

## Testing

1. Run the installer as Administrator
2. Launch from Start Menu
3. Check that `gpu_info.json` is created
4. Verify uninstall removes all files