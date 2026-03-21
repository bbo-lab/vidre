include(FetchContent)

# JSON
FetchContent_Declare(nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG        v3.12.0
    GIT_PROGRESS   ON
    FIND_PACKAGE_ARGS
)
FetchContent_MakeAvailable(nlohmann_json)

