# Package: nlohmann-json
# Version: 3.11
#
# Ubuntu: 
# - nlohmann-json3-dev

if(VIDRE_DEPS_CHECK_ONLY)
    find_package(nlohmann_json QUIET)
    vidre_deps_set(nlohmann-json ${nlohmann_json_FOUND} REQUIRED)
else()
    FetchContent_Declare(nlohmann_json
        GIT_REPOSITORY https://github.com/nlohmann/json.git
        GIT_TAG        v3.12.0
        GIT_SHALLOW    TRUE
        GIT_PROGRESS   ON
        FIND_PACKAGE_ARGS
    )

    # FIXME exported targets are not working - probably header only

    FetchContent_MakeAvailable(nlohmann_json)

    set(JSONLIB nlohmann_json::nlohmann_json)
    vidre_deps_set(nlohmann-json ON)
endif()

