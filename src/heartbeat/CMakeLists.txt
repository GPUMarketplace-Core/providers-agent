add_library(heartbeat STATIC
    src/heartbeat_client.cpp
)

target_include_directories(heartbeat
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(heartbeat
    PUBLIC common
    PRIVATE nlohmann_json::nlohmann_json
)

# Platform-specific HTTP library
if(WIN32)
    # Use WinHTTP on Windows
    target_link_libraries(heartbeat PRIVATE winhttp)
else()
    # Use libcurl on macOS/Linux for testing
    find_package(CURL REQUIRED)
    target_link_libraries(heartbeat PRIVATE CURL::libcurl)
endif()

# Threading support
find_package(Threads REQUIRED)
target_link_libraries(heartbeat PRIVATE Threads::Threads)
