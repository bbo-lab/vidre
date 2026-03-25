set(PLUGIN_OUTPUT_DIR "${CMAKE_BINARY_DIR}/plugins" CACHE INTERNAL "")

configure_file(devices.lua ${PLUGIN_OUTPUT_DIR}/devices.cfg COPYONLY)
install(FILES ${PLUGIN_OUTPUT_DIR}/devices.cfg
    DESTINATION ${CMAKE_INSTALL_PLUGINDIR}/
    COMPONENT "runtime")

add_subdirectory(camera)
