add_library(storage STATIC
    src/json_storage.cpp
)

target_include_directories(storage
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(storage
    PUBLIC common
    PRIVATE nlohmann_json::nlohmann_json
)

# Link filesystem library (needed for some compilers)
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS "9.0")
    target_link_libraries(storage PRIVATE stdc++fs)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS "9.0")
    target_link_libraries(storage PRIVATE c++fs)
endif()
