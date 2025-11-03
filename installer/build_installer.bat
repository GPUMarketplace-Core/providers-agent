@echo off
echo Building Providers Agent GPU Probe Installer...

REM Build the application first
cd ..
if not exist "build" mkdir build
cd build

echo Configuring CMake...
cmake .. -G "Visual Studio 17 2022" -A x64
if %ERRORLEVEL% neq 0 (
    echo CMake configuration failed!
    pause
    exit /b 1
)

echo Building application...
cmake --build . --config Release
if %ERRORLEVEL% neq 0 (
    echo Build failed!
    pause
    exit /b 1
)

echo Building installer...
cmake --build . --target installer --config Release
if %ERRORLEVEL% neq 0 (
    echo Installer build failed!
    pause
    exit /b 1
)

echo.
echo Installer built successfully!
echo Check installer\ directory for ProvidersAgent-GPUProbe-Setup.exe
pause