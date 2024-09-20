cmake_minimum_required(VERSION 3.18)

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

if(NOT DEFINED CMAKE_BUILD_TYPE_INIT)
    set(CMAKE_BUILD_TYPE_INIT Debug)
endif()

set(CMAKE_USER_MAKE_RULES_OVERRIDE "VidreInitFlags.cmake")

project(vidre
    VERSION 0.0.0
    LANGUAGES CXX
    DESCRIPTION "VidRe Recording Daemon."
    HOMEPAGE_URL https://github.com/bbo-lab/vidre
)

include(CheckIncludeFile)
include(CheckIncludeFileCXX)

if(NOT DEFINED EXECUTABLE_OUTPUT_PATH)
    set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
endif(NOT DEFINED EXECUTABLE_OUTPUT_PATH)

if(NOT DEFINED LIBRARY_OUTPUT_PATH)
    set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)
endif(NOT DEFINED LIBRARY_OUTPUT_PATH)

# COMPILER ####################################################################

add_compile_options(-Wzero-as-null-pointer-constant)
if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    add_compile_options(-Wno-vla-cxx-extension -ferror-limit=512)
endif()

# DEPENDENCIES ################################################################

# Threads
set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package(Threads)

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

add_subdirectory(src)
add_subdirectory(examples)
add_subdirectory(docs)

# TEST ########################################################################

# TODO 
# - setup dart
# - setup CMake for testing

add_subdirectory(tests)
