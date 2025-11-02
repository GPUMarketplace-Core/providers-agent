add_library(tray_agent STATIC
    src/tray_agent.cpp
)

target_include_directories(tray_agent
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(tray_agent
    PUBLIC common
)

if(WIN32)
    target_link_libraries(tray_agent PRIVATE shell32 user32)
endif()
