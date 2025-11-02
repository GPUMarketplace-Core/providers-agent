add_library(credential_manager STATIC
    src/credential_manager.cpp
)

target_include_directories(credential_manager
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(credential_manager
    PUBLIC common
)

# Windows Credential Manager and DPAPI
if(WIN32)
    target_link_libraries(credential_manager PRIVATE advapi32 crypt32)
endif()
