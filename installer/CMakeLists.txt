# Inno Setup installer configuration
if(WIN32)
    # Find Inno Setup compiler
    find_program(INNO_SETUP_COMPILER
        NAMES iscc ISCC
        PATHS
            "$ENV{ProgramFiles}\Inno Setup 6"
            "$ENV{ProgramFiles(x86)}\Inno Setup 6"
            "$ENV{ProgramFiles}\Inno Setup 5"
            "$ENV{ProgramFiles(x86)}\Inno Setup 5"
        DOC "Inno Setup Compiler"
    )

    if(INNO_SETUP_COMPILER)
        # Add custom target to build installer
        add_custom_target(installer
            COMMAND ${INNO_SETUP_COMPILER} "${CMAKE_CURRENT_SOURCE_DIR}/setup.iss"
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            COMMENT "Building Inno Setup installer..."
            DEPENDS providers_agent
        )
        message(STATUS "Inno Setup found: ${INNO_SETUP_COMPILER}")
    else()
        message(WARNING "Inno Setup not found. Install from https://jrsoftware.org/isinfo.php")
    endif()
endif()
