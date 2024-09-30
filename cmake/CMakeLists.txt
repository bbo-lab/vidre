set(VIDRE_CONFIG_CMAKE ${Vidre_BINARY_DIR}/cmake/VidreConfig.cmake)
configure_file(VidreConfig.in.cmake ${VIDRE_CONFIG_CMAKE} @ONLY)

set(VIDRE_CONFIG_VERSION_CMAKE ${Vidre_BINARY_DIR}/cmake/VidreConfigVersion.cmake)
configure_file(VidreConfigVersion.in.cmake ${VIDRE_CONFIG_VERSION_CMAKE} @ONLY)

if(NOT Vidre_PARENT_DIR)
    install(FILES ${VIDRE_CONFIG_CMAKE}
        DESTINATION ${CMAKE_INSTALL_CMAKEDIR}
        COMPONENT "development")
    install(FILES ${VIDRE_CONFIG_VERSION_CMAKE}
        DESTINATION ${CMAKE_INSTALL_CMAKEDIR}
        COMPONENT "development")
endif()

if(NOT TARGET uninstall)
    configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.in.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
        IMMEDIATE @ONLY)

    add_custom_target(uninstall
        COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)
endif()

install(FILES  VidreMacros.cmake
    COMPONENT development
    DESTINATION ${CMAKE_INSTALL_CMAKEDIR}
)

