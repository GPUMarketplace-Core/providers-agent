@echo off
REM Build script for Providers Agent on Windows

echo ================================================
echo   Providers Agent - Windows Build Script
echo ================================================
echo.

REM Check if build directory exists
if exist build (
    echo Build directory exists. Cleaning...
    rmdir /s /q build
)

echo Creating build directory...
mkdir build
cd build

echo.
echo Running CMake configuration...
cmake .. -G "Visual Studio 17 2022" -A x64

if %ERRORLEVEL% NEQ 0 (
    echo ERROR: CMake configuration failed!
    pause
    exit /b 1
)

echo.
echo Building Release configuration...
cmake --build . --config Release

if %ERRORLEVEL% NEQ 0 (
    echo ERROR: Build failed!
    pause
    exit /b 1
)

echo.
echo ================================================
echo   Build Complete!
echo ================================================
echo.
echo Executable location: build\bin\Release\providers_agent.exe
echo.
echo Usage:
echo   - Console mode: build\bin\Release\providers_agent.exe --console
echo   - GUI mode:     build\bin\Release\providers_agent.exe --gui
echo.
echo Run 'providers_agent.exe --help' for more options.
echo.
pause
