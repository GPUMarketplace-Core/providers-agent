cmake_minimum_required(VERSION 3.20)
project(ProvidersAgent VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Options
option(BUILD_TESTS "Build tests" ON)
option(BUILD_GUI "Build GUI application" ON)

# Third-party dependencies
add_subdirectory(third_party)

# Core modules
add_subdirectory(src/system_probe)
add_subdirectory(src/gpu_probe)
add_subdirectory(src/heartbeat)
add_subdirectory(src/credential_manager)
add_subdirectory(src/storage)
add_subdirectory(src/job_runner)
add_subdirectory(src/common)

# GUI Application
if(BUILD_GUI)
    add_subdirectory(src/gui)
    add_subdirectory(src/tray_agent)
endif()

# Main executable
add_subdirectory(src/app)

# Tests
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# Installer
add_subdirectory(installer)
