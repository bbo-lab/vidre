# Package: Catch2
# Version: 3.4
#
# Info: Only required for developers.
#
# Ubuntu:
# - catch2

set(Catch2_DEPENDENCY OPTIONAL)

if(VIDRE_DEPS_CHECK_ONLY)
    find_package(Catch2 QUIET
        3.4)
else(NOT Catch2_FOUND AND VIDRE_DEVELOP)
    # see https://github.com/catchorg/Catch2/blob/v3.7.1/docs/tutorial.md
    FetchContent_Declare(Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG        v3.7.1
        GIT_PROGRESS   ON
        FIND_PACKAGE_ARGS 3.4
    )

    FetchContent_MakeAvailable(Catch2)
endif()

vidre_deps_set(Catch2 ${Catch2_FOUND} ${Catch2_DEPENDENCY})
