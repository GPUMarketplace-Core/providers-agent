# Example programs for testing

add_executable(test_heartbeat
    test_heartbeat.cpp
)

target_link_libraries(test_heartbeat
    PRIVATE
        heartbeat
        common
)

add_executable(test_credentials
    test_credentials.cpp
)

target_link_libraries(test_credentials
    PRIVATE
        credential_manager
        common
)
