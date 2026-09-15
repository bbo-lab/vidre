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
include(CheckCXXCompilerFlag)
include(CheckIncludeFile)
include(CheckIncludeFileCXX)
include(CheckTypeSize)

include(VidreMacros)

# COMPILER ####################################################################
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

# OS ##########################################################################
# "Native" Windows build
if (WIN32 AND NOT CYGWIN AND NOT MINGW AND NOT MSYS)
    set(NATIVE_MSVC ON)
else()
    set(NATIVE_MSVC OFF)
endif()

# Enable large-file support only when the platform still uses 32-bit off_t.
set(CMAKE_EXTRA_INCLUDE_FILES sys/types.h)
check_type_size("off_t" SIZEOF_OFF_T LANGUAGE CXX)
unset(CMAKE_EXTRA_INCLUDE_FILES)

if(NOT SIZEOF_OFF_T)
    message(WARNING "Could not determine sizeof(off_t). Skipping _FILE_OFFSET_BITS check.")
elseif(SIZEOF_OFF_T LESS 8)
    set(CMAKE_EXTRA_INCLUDE_FILES sys/types.h)
    set(CMAKE_REQUIRED_DEFINITIONS -D_FILE_OFFSET_BITS=64)
    check_type_size("off_t" SIZEOF_OFF_T_WITH_FILE_OFFSET_BITS_64 LANGUAGE CXX)
    unset(CMAKE_REQUIRED_DEFINITIONS)
    unset(CMAKE_EXTRA_INCLUDE_FILES)

    if(SIZEOF_OFF_T_WITH_FILE_OFFSET_BITS_64 EQUAL 8)
        message(STATUS "Enabling _FILE_OFFSET_BITS=64 for 64-bit file offsets.")
        add_compile_definitions(_FILE_OFFSET_BITS=64)
    else()
        message(WARNING "off_t remains 32-bit even with _FILE_OFFSET_BITS=64.")
    endif()
else()
    message(STATUS "off_t is already ${SIZEOF_OFF_T} bytes; _FILE_OFFSET_BITS=64 is not required.")
endif()

# DEVELOPERS ##################################################################
set(VIDRE_DEVELOP OFF CACHE BOOL "Developer mode")

set(VIDRE_BUILD_EXAMPLES ${VIDRE_DEVELOP} CACHE BOOL "Build examples.")
set(VIDRE_BUILD_TESTS ${VIDRE_DEVELOP} CACHE BOOL "Build tests.")
set(VIDRE_BUILD_GUI ON CACHE BOOL "Build GUIs.")

set(VIDRE_FETCH_DEPS OFF CACHE BOOL "Fetch dependencies")
mark_as_advanced(VIDRE_FETCH_DEPS)

# DEPENDENCIES ################################################################
include(vidre/deps)

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
    "-DPROJECT_SOURCE_DIR=\"${PROJECT_SOURCE_DIR}\""
    "-DPROJECT_BINARY_DIR=\"${PROJECT_BINARY_DIR}\""
    "-DCMAKE_INSTALL_LUA_CDIR=\"${CMAKE_INSTALL_LUA_CDIR}\""
    "-DCMAKE_INSTALL_LUA_LDIR=\"${CMAKE_INSTALL_LUA_LDIR}\""
)

# BUILD #######################################################################

add_subdirectory(cmake)
add_subdirectory(icons)
add_subdirectory(share)
add_subdirectory(src)

# FIXME
# install(EXPORT VidreTargets-${Vidre_RELEASE}
#     DESTINATION ${CMAKE_INSTALL_CMAKEDIR}
#     COMPONENT "development"
# )

add_subdirectory(docs) 

install(FILES 
        LICENSE README.md
    DESTINATION ${CMAKE_INSTALL_DOCDIR}
    COMPONENT "runtime")

# TEST ########################################################################

if(NOT Vidre_IS_SUBPROJECT)
    if (VIDRE_BUILD_TESTS)
        enable_testing()
        find_program(CMAKE_MEMORYCHECK_COMMAND valgrind)

        if (DEFINED $ENV{VIDRE_TESTS_DART})
            # TODO 
            # - setup dart
        endif()

        add_subdirectory(tests)
    endif()

    if (VIDRE_BUILD_EXAMPLES)
        add_subdirectory(examples)
    endif()
endif()
