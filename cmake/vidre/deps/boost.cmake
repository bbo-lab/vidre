# Package: Boost
# Version: 1.83
# 
# Ubuntu: 
# - libboost-exception-dev
# - libboost-system-dev
# - libboost-iostreams-dev

include(vidre/deps/libuuid)
if(NOT VIDRE_HAVE_LIBUUID)
    set(BOOST_COMPONENT_UUID uuid)
    set(UUIDLIB Boost::uuid)
endif()

set(BOOST_VERSION 1.83.0)
set(BOOST_ENABLE_CMAKE ON)
set(Boost_USE_STATIC_LIBS OFF) 
set(Boost_USE_MULTITHREADED ON)  
set(Boost_USE_STATIC_RUNTIME OFF) 

if(VIDRE_DEPS_CHECK_ONLY)
    find_package(Boost QUIET
        ${BOOST_VERSION}
        COMPONENTS
            exception
            system
            iostreams
            ${BOOST_COMPONENT_UUID}
        CONFIG
    )
    vidre_deps_set(Boost ${Boost_FOUND} REQUIRED)
else()
    FetchContent_Declare(
        Boost
        GIT_REPOSITORY https://github.com/boostorg/boost.git
        GIT_TAG boost-1.90.0
        GIT_SHALLOW TRUE
        GIT_PROGRESS ON
        FIND_PACKAGE_ARGS
            ${BOOST_VERSION}
            COMPONENTS
                exception
                system
                iostreams
                ${BOOST_COMPONENT_UUID}
            CONFIG
    )
    FetchContent_MakeAvailable(Boost)
endif()
