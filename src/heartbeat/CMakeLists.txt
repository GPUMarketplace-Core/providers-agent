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

# Link HTTP library (WinHTTP on Windows, curl on macOS/Linux)
if(WIN32)
    target_link_libraries(heartbeat PRIVATE winhttp)
else()
    find_package(CURL REQUIRED)
    target_link_libraries(heartbeat PRIVATE CURL::libcurl)
endif()
