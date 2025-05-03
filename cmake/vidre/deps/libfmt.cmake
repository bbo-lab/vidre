# fmtlib

# C++-20 format library
check_include_file_cxx("format" VIDRE_HAVE_CPP_LIB_FORMAT)
if(NOT VIDRE_HAVE_CPP_LIB_FORMAT)
    find_package(fmt REQUIRED)
    set(FMTLIB fmt::fmt)
    set(VIDRE_HAVE_CPP_LIB_FORMAT 0)
endif()


# https://github.com/fmtlib/fmt.git