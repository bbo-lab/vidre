cmake_minimum_required(VERSION 3.25)

set(CMAKE_CXX_STANDARD 23)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

if(NOT DEFINED CMAKE_BUILD_TYPE_INIT)
    set(CMAKE_BUILD_TYPE_INIT RelWithDebInfo)
endif()

set(Vidre_ORGANIZATION_NAME "MPINB")
set(Vidre_ORGANIZATION_DOMAIN "de.mpg.mpinb")

if(NOT DEFINED CMAKE_INSTALL_PREFIX)
    if(WIN32)
        set(CMAKE_INSTALL_PREFIX "C:/Programs/${Vidre_ORGANIZATION_NAME}" CACHE PATH "Install path prefix, prepended onto install directories.")
    else()
        set(CMAKE_INSTALL_PREFIX "/opt/${Vidre_ORGANIZATION_NAME}" CACHE PATH "Install path prefix, prepended onto install directories.")
    endif()
endif()

set(CMAKE_USER_MAKE_RULES_OVERRIDE "VidreInitFlags.cmake")

project(Vidre
    VERSION 0.0.0
    LANGUAGES CXX
    DESCRIPTION "Video recording software for scientific purposes."
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

if(NOT DEFINED PLUGIN_OUTPUT_PATH)
    set(PLUGIN_OUTPUT_PATH ${PROJECT_BINARY_DIR}/plugins)
endif(NOT DEFINED PLUGIN_OUTPUT_PATH)

if(NOT DEFINED DATA_OUTPUT_PATH)
    set(DATA_OUTPUT_PATH ${PROJECT_BINARY_DIR}/share)
endif(NOT DEFINED DATA_OUTPUT_PATH)

include(GNUInstallDirs)

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

# DEPENDENCIES ################################################################
include(VidreFetchContent)

include(vidre/deps/boost)
include(vidre/deps/libfmt)
include(vidre/deps/json)
include(vidre/deps/lua)
include(vidre/deps/libuuid)
include(vidre/deps/units)
include(vidre/deps/terminal)
include(vidre/deps/ffmpeg)
include(vidre/deps/qt)

# PATHS #######################################################################

set(CMAKE_INSTALL_DATADIR ${CMAKE_INSTALL_DATAROOTDIR}/vidre)
set(CMAKE_INSTALL_DOCDIR ${CMAKE_INSTALL_DATAROOTDIR}/doc/vidre)

# FIXME see https://cmake.org/cmake/help/latest/command/find_package.html
set(CMAKE_INSTALL_CMAKEDIR ${CMAKE_INSTALL_DATAROOTDIR}/cmake/vidre) 

set(CMAKE_INSTALL_PLUGINDIR ${CMAKE_INSTALL_LIBDIR}/vidre/plugins)
set(CMAKE_INSTALL_SCRIPTDIR ${CMAKE_INSTALL_DATADIR}/scripts)

set(CMAKE_INSTALL_LUA_CDIR ${CMAKE_INSTALL_PREFIX}/${LUA_CDIR})
set(CMAKE_INSTALL_LUA_LDIR ${CMAKE_INSTALL_PREFIX}/${LUA_LDIR})

add_definitions(
    -DPROJECT_SOURCE_DIR=\"${PROJECT_SOURCE_DIR}\"
    -DPROJECT_BINARY_DIR=\"${PROJECT_BINARY_DIR}\"
    -DCMAKE_INSTALL_LUA_CDIR=\"${CMAKE_INSTALL_LUA_CDIR}\"
    -DCMAKE_INSTALL_LUA_LDIR=\"${CMAKE_INSTALL_LUA_LDIR}\"
)

# BUILD #######################################################################

add_subdirectory(cmake)
add_subdirectory(icons)
add_subdirectory(share)
add_subdirectory(src)

install(EXPORT VidreTargets-${Vidre_RELEASE}
    DESTINATION ${CMAKE_INSTALL_CMAKEDIR}
    COMPONENT "development"
)

add_subdirectory(docs)

install(FILES 
        LICENSE README.md
    DESTINATION ${CMAKE_INSTALL_DOCDIR}
    COMPONENT "runtime")


# TEST ########################################################################

if(NOT Vidre_IS_SUBPROJECT AND (CMAKE_BUILD_TYPE STREQUAL Debug))
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
