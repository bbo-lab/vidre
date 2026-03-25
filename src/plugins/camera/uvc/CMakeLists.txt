set(UVC_PLUGIN_PATH "camera/uvc")

vidre_module(uvc 
    PROPERTIES
        OUTPUT_NAME uvc
        LIBRARY_OUTPUT_DIRECTORY "${PLUGIN_OUTPUT_DIR}/${UVC_PLUGIN_PATH}"
    BUILD
        LINK_LIBRARIES
            PUBLIC 
                Vidre::Core
        INSTALL
            DESTINATION "${CMAKE_INSTALL_PLUGINDIR}/${UVC_PLUGIN_PATH}"
            COMPONENT "runtime"
    SOURCES
        uvc.cc
)

configure_file(plugin.lua ${PLUGIN_OUTPUT_DIR}/${UVC_PLUGIN_PATH}/plugin.cfg COPYONLY)
install(FILES ${PLUGIN_OUTPUT_DIR}/${UVC_PLUGIN_PATH}/plugin.cfg 
    DESTINATION ${CMAKE_INSTALL_PLUGINDIR}/${UVC_PLUGIN_PATH}
    COMPONENT "runtime")
