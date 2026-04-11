# Package: libfmt
# Version: 9.1.0
#
# Info: This package is only required if not <format> is not available or dysfunctional.
#
# Ubuntu:
# - libfmt-dev

# C++-20 format library
check_include_file_cxx("format" VIDRE_USE_CPP_LIB_FORMAT)

if(NOT VIDRE_USE_CPP_LIB_FORMAT)
    if(VIDRE_DEPS_CHECK_ONLY)
        find_package(fmt QUIET)
        vidre_deps_set(libfmt ${fmt_FOUND} REQUIRED)
    else()
        FetchContent_Declare(fmt
            GIT_REPOSITORY https://github.com/fmtlib/fmt.git
            GIT_TAG        12.1.0
            GIT_SHALLOW    TRUE
            GIT_PROGRESS   ON
            FIND_PACKAGE_ARGS
        )
        FetchContent_MakeAvailable(fmt)

        set(FMTLIB fmt::fmt)
    endif()
endif()
