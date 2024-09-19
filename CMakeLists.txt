cmake_minimum_required(VERSION 3.18)

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

if(NOT DEFINED CMAKE_BUILD_TYPE_INIT)
    set(CMAKE_BUILD_TYPE_INIT Debug)
endif()

set(CMAKE_USER_MAKE_RULES_OVERRIDE "VidreInitFlags.cmake")

project(vidre-service
    VERSION 0.0.0
    LANGUAGES CXX
    DESCRIPTION "VidRe Recording Daemon."
    HOMEPAGE_URL https://github.com/bbo-lab/vidre-service
)
