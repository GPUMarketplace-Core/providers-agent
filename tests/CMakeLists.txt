# Add Google Test
include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG release-1.12.1
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

include(GoogleTest)

# Add test executable
add_executable(unit_tests
    test_storage.cpp
    test_tray_agent.cpp
    # test_system_probe.cpp
    # test_gpu_probe.cpp
    # test_heartbeat.cpp
    # test_credential_manager.cpp
)

target_link_libraries(unit_tests
    PRIVATE
        GTest::gtest_main
        storage
        tray_agent
)

target_include_directories(unit_tests
    PRIVATE
        ${CMAKE_SOURCE_DIR}/src/storage/include
        ${CMAKE_SOURCE_DIR}/src/tray_agent/include
        ${CMAKE_SOURCE_DIR}/src/common/include
)

# Discover tests
gtest_discover_tests(unit_tests)
