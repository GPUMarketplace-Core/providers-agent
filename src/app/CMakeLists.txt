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

if(WIN32)
    # Set as Windows GUI application (not console)
    # set_target_properties(providers_agent PROPERTIES WIN32_EXECUTABLE TRUE)
endif()
