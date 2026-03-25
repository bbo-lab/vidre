find_package(PYLON 8 QUIET)
if(PYLON_FOUND)
    message(STATUS "Using pylon SDK (Basler AG)")
    set(VIDRE_HAVE_PYLON TRUE CACHE BOOL "Basler pylon library and includes found")
else()
    set(VIDRE_HAVE_PYLON FALSE CACHE BOOL "Basler pylon library and includes found")
endif()

if(VIDRE_HAVE_PYLON)
    set(PYLON_PLUGIN_PATH "camera/basler/pylon")

    vidre_module(pylon
        PROPERTIES
            OUTPUT_NAME pylon
            LIBRARY_OUTPUT_DIRECTORY "${PLUGIN_OUTPUT_DIR}/${PYLON_PLUGIN_PATH}"
        BUILD
            LINK_LIBRARIES
                PUBLIC 
                    Vidre::Core
                    pylon::PylonBase
                    GenICam::GCBase 
                    GenICam::GenApi
                    # pylon::PylonUtility ??
            INSTALL
                DESTINATION "${CMAKE_INSTALL_PLUGINDIR}/${PYLON_PLUGIN_PATH}"
                COMPONENT "runtime"
        SOURCES
            pylon.cc
    )

    configure_file(plugin.lua ${PLUGIN_OUTPUT_DIR}/${PYLON_PLUGIN_PATH}/plugin.cfg COPYONLY)
    install(FILES ${PLUGIN_OUTPUT_DIR}/${PYLON_PLUGIN_PATH}/plugin.cfg 
        DESTINATION ${CMAKE_INSTALL_PLUGINDIR}/${PYLON_PLUGIN_PATH}
        COMPONENT "runtime")
endif()

