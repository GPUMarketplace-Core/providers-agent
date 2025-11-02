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
