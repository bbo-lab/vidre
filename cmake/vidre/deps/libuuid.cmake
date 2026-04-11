# Package: uuid
# Version: 2.39.3
#
# Ubuntu: 
# - uuid-dev
#

if(PkgConfig_FOUND)
    pkg_check_modules(LIBUUID IMPORTED_TARGET uuid)
    if(TARGET PkgConfig::LIBUUID)
        set(UUIDLIB PkgConfig::LIBUUID)
        set(VIDRE_HAVE_LIBUUID ON CACHE INTERNAL "")
    endif()
endif()

if(NOT VIDRE_HAVE_LIBUUID)
    check_include_file_cxx(uuid/uuid.h VIDRE_HAVE_LIBUUID)
    if(VIDRE_HAVE_LIBUUID)
        set(UUIDLIB uuid)
    endif()
endif()

vidre_deps_set(uuid ${VIDRE_HAVE_LIBUUID} RECOMMENDED)
