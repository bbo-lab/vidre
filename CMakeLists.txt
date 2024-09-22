cmake_minimum_required(VERSION 3.18)

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

if(NOT DEFINED CMAKE_BUILD_TYPE_INIT)
    set(CMAKE_BUILD_TYPE_INIT Debug)
endif()

set(VIDRE_ORGANIZATION_NAME "MPINB")
set(VIDRE_ORGANIZATION_DOMAIN "org.mpg.mpinb")

if(NOT DEFINED CMAKE_INSTALL_PREFIX)
    if(WIN32)
    set(CMAKE_INSTALL_PREFIX "C:\\Programs\\${VIDRE_ORGANIZATION_NAME}" CACHE PATH "Install path prefix, prepended onto install directories.")
    else()
        set(CMAKE_INSTALL_PREFIX "/opt/${VIDRE_ORGANIZATION_NAME}" CACHE PATH "Install path prefix, prepended onto install directories.")
    endif()
endif()

set(CMAKE_USER_MAKE_RULES_OVERRIDE "VidreInitFlags.cmake")

project(Vidre
    VERSION 0.0.0
    LANGUAGES CXX
    DESCRIPTION "VidRe Recording Daemon."
    HOMEPAGE_URL https://github.com/bbo-lab/vidre
)

set(Vidre_RELEASE ${Vidre_VERSION_MAJOR}.${Vidre_VERSION_MINOR})

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

include(CheckIncludeFile)
include(CheckIncludeFileCXX)

if(NOT DEFINED EXECUTABLE_OUTPUT_PATH)
    set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
endif(NOT DEFINED EXECUTABLE_OUTPUT_PATH)

if(NOT DEFINED LIBRARY_OUTPUT_PATH)
    set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)
endif(NOT DEFINED LIBRARY_OUTPUT_PATH)

include(GNUInstallDirs)
set(CMAKE_INSTALL_DATADIR ${CMAKE_INSTALL_DATAROOTDIR}/vidre)

include(VidreMacros)

# COMPILER ####################################################################

# DEPENDENCIES ################################################################

# UUID
check_include_file_cxx("uuid/uuid.h" HAVE_LIBUUID)
if(NOT HAVE_LIBUUID)
    message(FATAL "Could not find <uuid/uuid.h>")
endif()
set(UUIDLIB uuid)

# JSON
find_package(nlohmann_json REQUIRED)
# FIXME Fetch if not available on system

# BUILD #######################################################################

add_subdirectory(cmake)
add_subdirectory(src)
# add_subdirectory(examples)
add_subdirectory(docs)

install(EXPORT VidreTargets-${Vidre_RELEASE}
    COMPONENT "development"
    DESTINATION share/cmake/vidre-${Vidre_RELEASE} 
)

# TEST ########################################################################

# TODO 
# - setup dart
# - setup CMake for testing

add_subdirectory(tests)
