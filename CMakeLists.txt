cmake_minimum_required(VERSION 3.25)

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

if(NOT DEFINED CMAKE_BUILD_TYPE_INIT)
    set(CMAKE_BUILD_TYPE_INIT RelWithDebInfo)
endif()

set(Vidre_ORGANIZATION_NAME "MPINB")
set(Vidre_ORGANIZATION_DOMAIN "org.mpg.mpinb")

if(NOT DEFINED CMAKE_INSTALL_PREFIX)
    if(WIN32)
    set(CMAKE_INSTALL_PREFIX "C:\\Programs\\${Vidre_ORGANIZATION_NAME}" CACHE PATH "Install path prefix, prepended onto install directories.")
    else()
        set(CMAKE_INSTALL_PREFIX "/opt/${Vidre_ORGANIZATION_NAME}" CACHE PATH "Install path prefix, prepended onto install directories.")
    endif()
endif()

set(CMAKE_USER_MAKE_RULES_OVERRIDE "VidreInitFlags.cmake")

if(NOT DEFINED PROJECT_NAME)
    set(Vidre_IS_SUBPROJECT OFF)
else()
    set(Vidre_IS_SUBPROJECT ON)
endif()

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
set(CMAKE_INSTALL_CMAKEDIR share/cmake/vidre) # FIXME system dependent, see https://cmake.org/cmake/help/latest/command/find_package.html

include(VidreMacros)

# COMPILER ####################################################################
include(CheckCXXCompilerFlag)
include(CheckIncludeFile)
include(CheckIncludeFileCXX)

# sanitize undefined behavior (UB)
if(CMAKE_BUILD_TYPE STREQUAL Debug)
    # compile and link with sanitizer
    check_cxx_compiler_flag(-fsanitize=undefined sanitize_undefind_behavior)
    if(sanitize_undefind_behavior)
        add_compile_options(-fsanitize=undefined -DSANITIZE_UB)
        add_link_options(-fsanitize=undefined)
    endif()

    check_cxx_compiler_flag(-fsanitize-link-c++-runtime sanitize_link_runtime)
    if(sanitize_link_runtime)
        add_link_options(-fsanitize-link-c++-runtime)
    endif()

    # trap on UB
    check_cxx_compiler_flag(-fno-sanitize-recover=undefined no_sanitize_recover)
    if(no_sanitize_recover)
        add_compile_options(-fno-sanitize-recover=undefined)
    endif()
endif()

# C++-20 format library
check_include_file_cxx("format" VIDRE_HAVE_CPP_LIB_FORMAT)
if(NOT VIDRE_HAVE_CPP_LIB_FORMAT)
    find_package(fmt REQUIRED)
    set(FMTLIB fmt::fmt)
    set(VIDRE_HAVE_CPP_LIB_FORMAT 0)
endif()

# DEPENDENCIES ################################################################
include(FetchContent)

# Qt
find_package(Qt6 6.0 REQUIRED
    COMPONENTS Core Gui Network Widgets)

set(CMAKE_AUTOMOC OFF)
set(CMAKE_AUTORCC OFF)
set(CMAKE_AUTOUIC OFF)

# Boost
set(BOOST_ENABLE_CMAKE ON)
# set(Boost_USE_STATIC_LIBS OFF) 
# set(Boost_USE_MULTITHREADED ON)  
# set(Boost_USE_STATIC_RUNTIME OFF) 

FetchContent_Declare(
  Boost
  GIT_REPOSITORY https://github.com/boostorg/boost.git
  GIT_TAG boost-1.88.0
  GIT_SHALLOW TRUE
  FIND_PACKAGE_ARGS
        CONFIG
        COMPONENTS 
            exception
            system
            uuid
)
FetchContent_MakeAvailable(Boost)

# UUID
check_include_file_cxx("uuid/uuid.h" VIDRE_HAVE_LIBUUID)
if(VIDRE_HAVE_LIBUUID)
    set(UUIDLIB uuid)
else()
    message(FATAL "Could not find <uuid/uuid.h>")
endif()

# JSON
FetchContent_Declare(nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG        v3.11.3
    GIT_PROGRESS   ON
    FIND_PACKAGE_ARGS
)
FetchContent_MakeAvailable(nlohmann_json)

# SI-Units
set(MP_UNITS_API_CONTRACTS NONE)
# set(MP_UNITS_BUILD_CXX_MODULES ON)
FetchContent_Declare(mp-units
  GIT_REPOSITORY https://github.com/mpusz/mp-units
  GIT_TAG 2.2
  SOURCE_SUBDIR src
)
FetchContent_MakeAvailable(mp-units)
set(HAVE_UNITS_MP_UNITS 1)
set(UNITSLIB mp-units)

# ffmpeg
find_package(FFmpeg COMPONENTS AVCODEC AVFORMAT AVUTIL AVDEVICE)

# BUILD #######################################################################

add_subdirectory(cmake)
add_subdirectory(src)

install(EXPORT VidreTargets-${Vidre_RELEASE}
    COMPONENT "development"
    DESTINATION ${CMAKE_INSTALL_CMAKEDIR}
)

add_subdirectory(docs)

# TEST ########################################################################

if(NOT Vidre_IS_SUBPROJECT)
    enable_testing()
    find_program(CMAKE_MEMORYCHECK_COMMAND valgrind)

    # TODO 
    # - setup dart

    # Catch2
    # see https://github.com/catchorg/Catch2/blob/v3.7.1/docs/tutorial.md
    FetchContent_Declare(Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG        v3.7.1
        GIT_PROGRESS   ON
        FIND_PACKAGE_ARGS 3.7
    )
    FetchContent_MakeAvailable(Catch2)

    add_subdirectory(examples)
    add_subdirectory(tests)
endif()
