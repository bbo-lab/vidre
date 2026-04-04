include(FetchContent)

FetchContent_Declare(
    yaml-cpp
    GIT_REPOSITORY https://github.com/jbeder/yaml-cpp.git
    GIT_TAG yaml-cpp-0.9.0
    GIT_SHALLOW TRUE
    GIT_PROGRESS ON
    FIND_PACKAGE_ARGS
)

set(YAML_BUILD_SHARED_LIBS ON)
set(YAML_CPP_INSTALL ON)

FetchContent_MakeAvailable(yaml-cpp)

set(YAMLLIB yaml-cpp)