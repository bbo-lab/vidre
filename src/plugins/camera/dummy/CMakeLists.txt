set(PLUGIN_CURRENT_DIR "camera/dummy")

vidre_module(dummy 
    PROPERTIES
        OUTPUT_NAME dummy
        LIBRARY_OUTPUT_DIRECTORY "${PLUGIN_OUTPUT_DIR}/${PLUGIN_CURRENT_DIR}"
    BUILD
        LINK_LIBRARIES
            PUBLIC 
                Vidre::Core
        INSTALL
            DESTINATION "${CMAKE_INSTALL_PLUGINDIR}/${PLUGIN_CURRENT_DIR}"
            COMPONENT "runtime"
    SOURCES
        dummy.cc
)

configure_file(plugin.lua ${PLUGIN_OUTPUT_DIR}/${PLUGIN_CURRENT_DIR}/plugin.cfg COPYONLY)
install(FILES plugin.lua
    RENAME plugin.cfg
    DESTINATION ${CMAKE_INSTALL_PLUGINDIR}/${PLUGIN_CURRENT_DIR}
    COMPONENT "runtime")
