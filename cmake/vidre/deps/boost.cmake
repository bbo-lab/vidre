include(FetchContent)

include(vidre/deps/libuuid)
if(NOT VIDRE_HAVE_LIBUUID)
  set(UUIDLIB Boost::uuid)
  set(BOOST_COMPONENT_UUID uuid)
endif()

# Boost
set(BOOST_ENABLE_CMAKE ON)
set(Boost_USE_STATIC_LIBS OFF) 
set(Boost_USE_MULTITHREADED ON)  
set(Boost_USE_STATIC_RUNTIME OFF) 

FetchContent_Declare(
  Boost
  GIT_REPOSITORY https://github.com/boostorg/boost.git
  GIT_TAG boost-1.90.0
  GIT_SHALLOW TRUE
  GIT_PROGRESS ON
  FIND_PACKAGE_ARGS
        CONFIG
        COMPONENTS 
            exception
            system
            ${BOOST_COMPONENT_UUID}
)
FetchContent_MakeAvailable(Boost)

