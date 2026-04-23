option(Vidre_BUILD_PLUGIN_CAMERA_UVC "Build the UVC camera plugin" ON)

if (Vidre_BUILD_PLUGIN_CAMERA_UVC)
    add_subdirectory(lib)
endif()

if (Vidre_BUILD_PLUGIN_CAMERA_UVC)
# set(PLUGIN_CURRENT_DIR "camera/uvc")

# vidre_module(uvc 
#     PROPERTIES
#         OUTPUT_NAME uvc
#         LIBRARY_OUTPUT_DIRECTORY "${PLUGIN_OUTPUT_PATH}/${PLUGIN_CURRENT_DIR}"
#     BUILD
#         LINK_LIBRARIES
#             PUBLIC 
#                 Vidre::Core
#         INSTALL
#             DESTINATION "${CMAKE_INSTALL_PLUGINDIR}/${PLUGIN_CURRENT_DIR}"
#             COMPONENT "runtime"
#     SOURCES
#         uvc.cc
# )

# install(FILES plugin.lua
#     RENAME plugin.cfg
#     DESTINATION ${CMAKE_INSTALL_PLUGINDIR}/${PLUGIN_CURRENT_DIR}
#     COMPONENT "runtime")

# configure_file(plugin.lua ${PLUGIN_OUTPUT_PATH}/${PLUGIN_CURRENT_DIR}/plugin.cfg COPYONLY)
endif()
