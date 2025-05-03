include(FetchContent)

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

