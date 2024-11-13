vidre_library(vidreffmpeg SHARED
    ALIAS Vidre::FFmpeg
    EXPORT VidreTargets-${Vidre_RELEASE}
    VERSION ${Vidre_VERSION}
    RELEASE ${Vidre_RELEASE}
    BUILD
        COMPILE_DEFINITIONS
            PRIVATE 
                VIDRE_FFMPEG_VISIBILITY=VIDRE_API_EXPORT
                ${FFMPEG_DEFINITIONS}
        COMPILE_FEATURES
            PUBLIC 
                cxx_std_23
        INCLUDE_DIRECTORIES
            PUBLIC
                $<INSTALL_INTERFACE:${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_INCLUDEDIR}>
                $<BUILD_INTERFACE:${vidre_build_include_dir}>
                $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
                $<BUILD_INTERFACE:${FFMPEG_INCLUDE_DIRS}>
        LINK_LIBRARIES
            PUBLIC
                ${FFMPEG_LIBRARIES}
                Vidre::Core
    INSTALL 
        LIBRARY COMPONENT runtime
        ARCHIVE COMPONENT development
        PUBLIC_HEADER COMPONENT development        
    SOURCES
        vidre/ffmpeg.hh vidre/ffmpeg.cc
        vidre/ffmpeg/config.hh vidre/ffmpeg/config.cc
        vidre/ffmpeg/util.hh
            vidre/ffmpeg/util/exception.hh vidre/ffmpeg/util/exception.cc
            vidre/ffmpeg/util/dictionary.hh
            vidre/ffmpeg/util/pixelformat.hh
            vidre/ffmpeg/util/frame.hh
)

message("FFMPEG_LIBRARIES=${FFMPEG_LIBRARIES}")