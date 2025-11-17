add_library(gui STATIC
    src/main_window.cpp
)

target_include_directories(gui
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(gui
    PUBLIC common
)

# Use Qt for cross-platform GUI
find_package(Qt6 COMPONENTS Core Widgets REQUIRED)
target_link_libraries(gui PRIVATE Qt6::Core Qt6::Widgets)

# Enable Qt MOC if needed
set(CMAKE_AUTOMOC ON)
