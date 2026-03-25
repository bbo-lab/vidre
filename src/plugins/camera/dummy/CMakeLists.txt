set(DUMMY_PLUGIN_PATH "camera/dummy")

vidre_module(dummy 
    PROPERTIES
        OUTPUT_NAME dummy
        LIBRARY_OUTPUT_DIRECTORY "${PLUGIN_OUTPUT_DIR}/${DUMMY_PLUGIN_PATH}"
    BUILD
        LINK_LIBRARIES
            PUBLIC 
                Vidre::Core
        INSTALL
            DESTINATION "${CMAKE_INSTALL_PLUGINDIR}/${DUMMY_PLUGIN_PATH}"
            COMPONENT "runtime"
    SOURCES
        dummy.cc
)

configure_file(plugin.lua ${PLUGIN_OUTPUT_DIR}/${DUMMY_PLUGIN_PATH}/plugin.cfg COPYONLY)
install(FILES ${PLUGIN_OUTPUT_DIR}/${DUMMY_PLUGIN_PATH}/plugin.cfg 
    DESTINATION ${CMAKE_INSTALL_PLUGINDIR}/${DUMMY_PLUGIN_PATH}
    COMPONENT "runtime")
