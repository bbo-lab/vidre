find_package(PYLON 8 QUIET)
if(PYLON_FOUND)
    message(STATUS "Using pylon SDK (Basler AG)")
    set(VIDRE_HAVE_PYLON TRUE CACHE BOOL "Basler pylon library and includes found")
else()
    set(VIDRE_HAVE_PYLON FALSE CACHE BOOL "Basler pylon library and includes found")
endif()

if(VIDRE_HAVE_PYLON)
    vidre_module(basler-pylon-module
        PROPERTIES
            OUTPUT_NAME module
            LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/module/basler/pylon
        BUILD
            LINK_LIBRARIES
                PUBLIC 
                    Vidre::Core
                    # GenICam::GCBase GenICam::GenApi
                    pylon::PylonBase
                    # pylon::PylonUtility
        SOURCES
            module.cc
    )
endif()

