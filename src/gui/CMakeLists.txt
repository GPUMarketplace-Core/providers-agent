add_library(gui STATIC
    src/main_window.cpp
)

target_include_directories(gui
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(gui
    PUBLIC common
    PUBLIC system_probe
)

# Win32 API implementation
if(WIN32)
    target_link_libraries(gui PRIVATE user32 gdi32)
endif()

# For Qt (uncomment when using Qt):
# find_package(Qt6 COMPONENTS Widgets REQUIRED)
# target_link_libraries(gui PRIVATE Qt6::Widgets)
