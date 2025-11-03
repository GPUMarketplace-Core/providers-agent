# nlohmann/json
include(FetchContent)

FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.12.0
)

FetchContent_MakeAvailable(nlohmann_json)

# TODO: Add other dependencies as needed
# - Google Test for testing
# - libcurl for HTTP (alternative to WinHTTP)
# - Qt if using Qt for GUI
