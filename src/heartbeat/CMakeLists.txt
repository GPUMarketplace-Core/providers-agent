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

# TODO: Link HTTP library (WinHTTP or curl)
if(WIN32)
    target_link_libraries(heartbeat PRIVATE winhttp)
endif()
