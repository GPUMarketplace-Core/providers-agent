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
