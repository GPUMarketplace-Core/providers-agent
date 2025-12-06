# TODO: Add Google Test
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG release-1.12.1
)
FetchContent_MakeAvailable(googletest)

include(GoogleTest)

if(WIN32)
    add_executable(unit_tests
        test_system_probe.cpp
        test_gpu_probe.cpp
    )
else()
    add_executable(unit_tests
        test_system_probe.cpp
    )
endif()

if(WIN32)
    target_link_libraries(unit_tests
        PRIVATE
            gtest_main
            system_probe
            gpu_probe
    )
else()
    target_link_libraries(unit_tests
        PRIVATE
            gtest_main
            system_probe
    )
endif()

gtest_discover_tests(unit_tests)

# Simple standalone tests (job runner)

find_package(Qt6 COMPONENTS Core REQUIRED)

add_executable(test_job_runner
    test_job_runner.cpp
)

target_link_libraries(test_job_runner
    PRIVATE
        job_runner
        Qt6::Core
)

# Ensure it runs in the same bin directory as the main app so it finds the blender folder
set_target_properties(test_job_runner PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# Ensure providers_agent is built so that the blender files are copied
add_dependencies(test_job_runner providers_agent)

# Heartbeat client test
add_executable(test_heartbeat
    ../examples/test_heartbeat.cpp
)

target_link_libraries(test_heartbeat
    PRIVATE
        heartbeat
)

set_target_properties(test_heartbeat PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)