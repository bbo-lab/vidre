# fmtlib

# C++-20 format library
check_include_file_cxx("format" VIDRE_USE_CPP_LIB_FORMAT)
if(NOT VIDRE_USE_CPP_LIB_FORMAT)
    set(VIDRE_USE_CPP_LIB_FORMAT 0)

    find_package(fmt REQUIRED)
    set(FMTLIB fmt::fmt)
    # FIXME GIT https://github.com/fmtlib/fmt.git
endif()


