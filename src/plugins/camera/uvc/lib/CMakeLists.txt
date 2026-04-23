## Linux
set(VidreDeviceUVC_SOURCES_Linux
    vidre/uvc/linux/dummy.hh
)

if (NOT DEFINED VidreDeviceUVC_SOURCES_${CMAKE_SYSTEM_NAME})
    message(NOTICE "Unsupported platform for UVC device (${CMAKE_SYSTEM_NAME}): disabling plugin.")
    set(Vidre_BUILD_PLUGIN_CAMERA_UVC OFF CACHE BOOL "Build the UVC camera plugin" FORCE)
endif()

if (Vidre_BUILD_PLUGIN_CAMERA_UVC)
    vidre_library(vidredev-uvc SHARED
        ALIAS Vidre::Device::UVC
        EXPORT VidreTargets-${Vidre_RELEASE}
        VERSION ${Vidre_VERSION}
        RELEASE ${Vidre_RELEASE}
        BUILD
            COMPILE_DEFINITIONS
                PRIVATE 
                    VIDRE_UVC_VISIBILITY=VIDRE_API_EXPORT
                    VIDRE_BINARY_DIR="${VIDRE_BINARY_DIR}"
                    VIDRE_SOURCE_DIR="${VIDRE_SOURCE_DIR}"
            COMPILE_FEATURES
                PUBLIC 
                    cxx_std_23
            INCLUDE_DIRECTORIES
                PUBLIC
                    $<INSTALL_INTERFACE:${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_INCLUDEDIR}>
                    $<BUILD_INTERFACE:${vidre_build_include_dir}>
                    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
            LINK_LIBRARIES
                PUBLIC
                    Vidre::Core
                    Vidre::Video
                    Vidre::Device
        INSTALL 
            LIBRARY COMPONENT runtime
            ARCHIVE COMPONENT development
            PUBLIC_HEADER COMPONENT development        
        SOURCES
            vidre/uvc/config.hh vidre/uvc/config.cc
            vidre/uvc/device.hh vidre/uvc/device.cc
            ${VidreDeviceUVC_SOURCES_${CMAKE_SYSTEM_NAME}}
    )
endif()