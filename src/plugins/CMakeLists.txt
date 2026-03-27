configure_file(devices.lua ${PLUGIN_OUTPUT_PATH}/devices.cfg COPYONLY)
install(FILES ${PLUGIN_OUTPUT_PATH}/devices.cfg
    DESTINATION ${CMAKE_INSTALL_PLUGINDIR}/
    COMPONENT "runtime")

add_subdirectory(camera)
