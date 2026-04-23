vidre_library(vidrevideo SHARED
    ALIAS Vidre::Video
    EXPORT VidreTargets-${Vidre_RELEASE}
    VERSION ${Vidre_VERSION}
    RELEASE ${Vidre_RELEASE}
    BUILD
        COMPILE_DEFINITIONS
            PRIVATE 
                VIDRE_VIDEO_VISIBILITY=VIDRE_API_EXPORT
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
    INSTALL 
        LIBRARY COMPONENT runtime
        ARCHIVE COMPONENT development
        PUBLIC_HEADER COMPONENT development        
    SOURCES
        vidre/video/config.hh vidre/video/config.cc
)
