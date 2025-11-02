# TODO: Add Google Test
# FetchContent_Declare(
#     googletest
#     GIT_REPOSITORY https://github.com/google/googletest.git
#     GIT_TAG release-1.12.1
# )
# FetchContent_MakeAvailable(googletest)
#
# include(GoogleTest)
#
# add_executable(unit_tests
#     test_system_probe.cpp
#     test_gpu_probe.cpp
#     test_heartbeat.cpp
#     test_credential_manager.cpp
#     test_storage.cpp
# )
#
# target_link_libraries(unit_tests
#     PRIVATE
#         gtest_main
#         system_probe
#         gpu_probe
#         heartbeat
#         credential_manager
#         storage
# )
#
# gtest_discover_tests(unit_tests)
