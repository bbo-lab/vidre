# Package: yaml-cpp
# Version: 0.8
#
# Ubuntu:
# - libyaml-cpp-dev

if(VIDRE_DEPS_CHECK_ONLY)
    find_package(yaml-cpp QUIET)
    vidre_deps_set(yaml-cpp ${yaml-cpp_FOUND} REQUIRED)
    if(yaml-cpp_FOUND)
        set(YAMLLIB yaml-cpp::yaml-cpp)
    endif()
else()
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

    set(YAMLLIB yaml-cpp::yaml-cpp)
endif()



