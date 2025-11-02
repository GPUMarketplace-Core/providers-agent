add_library(gpu_probe STATIC
    src/gpu_probe.cpp
)

target_include_directories(gpu_probe
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(gpu_probe
    PUBLIC common
    PRIVATE nlohmann_json::nlohmann_json
)

# TODO: Link NVML library when CUDA Toolkit is installed
# find_package(CUDAToolkit REQUIRED)
# target_link_libraries(gpu_probe PRIVATE CUDA::nvml)
