add_executable(providers_agent
    main.cpp
)

target_link_libraries(providers_agent
    PRIVATE
        common
        system_probe
        gpu_probe
        heartbeat
        credential_manager
        storage
        gui
        tray_agent
)

# Link Qt6 for GUI
find_package(Qt6 COMPONENTS Core Widgets REQUIRED)
target_link_libraries(providers_agent PRIVATE Qt6::Core Qt6::Widgets)
set(CMAKE_AUTOMOC ON)

if(WIN32)
    # Set as Windows GUI application (not console)
    # set_target_properties(providers_agent PROPERTIES WIN32_EXECUTABLE TRUE)
endif()
