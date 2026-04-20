# Package: FFmpeg
# Version: 6.1.1
#
# Ubuntu: 
# - libavcodec-dev
# - libavformat-dev
# - libavutil-dev
# - libavdevice-dev

if(VIDRE_DEPS_CHECK_ONLY)
    find_package(FFmpeg QUIET
        COMPONENTS 
            AVCODEC 
            AVFORMAT 
            AVUTIL 
            AVDEVICE
    )
    vidre_deps_set(FFmpeg ${FFmpeg_FOUND} REQUIRED)
else()
    FetchContent_Declare(FFmpeg
        GIT_REPOSITORY https://git.ffmpeg.org/ffmpeg.git
        GIT_TAG n6.1.4
        GIT_SHALLOW TRUE
        GIT_PROGRESS ON
        FIND_PACKAGE_ARGS
                COMPONENTS 
                    AVCODEC 
                    AVFORMAT 
                    AVUTIL 
                    AVDEVICE
    )
    FetchContent_MakeAvailable(FFmpeg)
endif()