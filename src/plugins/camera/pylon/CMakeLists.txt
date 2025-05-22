find_package(PYLON 8 QUIET)
if(PYLON_FOUND)
    message(STATUS "Using pylon SDK (Basler AG)")
    set(VIDRE_HAVE_PYLON TRUE CACHE BOOL "Basler pylon library and includes found")
else()
    set(VIDRE_HAVE_PYLON FALSE CACHE BOOL "Basler pylon library and includes found")
endif()

if(VIDRE_HAVE_PYLON)
    vidre_module(pylon-preload
        PROPERTIES
            OUTPUT_NAME preload
            LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/plugins/basler/pylon
        BUILD
            LINK_LIBRARIES
                PUBLIC 
                    Vidre::Core
                    # GenICam::GCBase GenICam::GenApi
                    pylon::PylonBase
                    # pylon::PylonUtility
        SOURCES
            preload.cc
    )
endif()

