add_library(system_probe STATIC
    src/system_probe.cpp
)

target_include_directories(system_probe
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(system_probe
    PUBLIC common
    PRIVATE nlohmann_json::nlohmann_json
)

# Windows specific libraries
if(WIN32)
    target_link_libraries(system_probe PRIVATE kernel32)
endif()
