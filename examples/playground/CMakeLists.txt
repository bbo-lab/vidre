vidre_test(GROUP 
    NAME playground
    BUILD
        COMPILE_DEFINITIONS
            PRIVATE
                Vidre_CURRENT_BINARY_DIR=\"${CMAKE_CURRENT_BINARY_DIR}\"
                Vidre_CURRENT_SOURCE_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}\"
        LINK_LIBRARIES 
            PRIVATE 
                Vidre::Core
)

find_package(FFmpeg COMPONENTS AVCODEC AVFORMAT AVUTIL AVDEVICE REQUIRED) #add here the list of ffmpeg components required

vidre_test(decode_video 
    BUILD
        COMPILE_DEFINITIONS
            PRIVATE
                ${FFMPEG_DEFINITIONS}
        INCLUDE_DIRECTORIES
            PUBLIC 
                ${FFMPEG_INCLUDE_DIRS}
        LINK_LIBRARIES 
            PUBLIC
                ${FFMPEG_LIBRARIES}
    
    SOURCES decode_video.cc
)
