find_package(PYLON 8 QUIET)
if(PYLON_FOUND)
    message(STATUS "Using pylon SDK (Basler AG)")
    set(VIDRE_HAVE_PYLON TRUE CACHE BOOL "Basler pylon library and includes found")
else()
    set(VIDRE_HAVE_PYLON FALSE CACHE BOOL "Basler pylon library and includes found")
endif()

if(VIDRE_HAVE_PYLON)
    set(PLUGIN_CURRENT_DIR "camera/basler/pylon")

    vidre_module(pylon
        PROPERTIES
            OUTPUT_NAME pylon
            LIBRARY_OUTPUT_DIRECTORY "${PLUGIN_OUTPUT_DIR}/${PLUGIN_CURRENT_DIR}"
        BUILD
            LINK_LIBRARIES
                PUBLIC 
                    Vidre::Core
                    pylon::PylonBase
                    GenICam::GCBase 
                    GenICam::GenApi
                    # pylon::PylonUtility ??
            INSTALL
                DESTINATION "${CMAKE_INSTALL_PLUGINDIR}/${PLUGIN_CURRENT_DIR}"
                COMPONENT "runtime"
        SOURCES
            pylon.cc
    )

    configure_file(plugin.lua ${PLUGIN_OUTPUT_DIR}/${PLUGIN_CURRENT_DIR}/plugin.cfg COPYONLY)
    install(FILES plugin.lua
        RENAME plugin.cfg
        DESTINATION ${CMAKE_INSTALL_PLUGINDIR}/${PLUGIN_CURRENT_DIR}
        COMPONENT "runtime")
endif()

