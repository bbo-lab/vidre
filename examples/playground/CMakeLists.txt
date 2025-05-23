vidre_test(GROUP 
    NAME playground
    BUILD
        COMPILE_FEATURES
            PUBLIC 
                cxx_std_23    
        COMPILE_DEFINITIONS
            PRIVATE
                Vidre_CURRENT_BINARY_DIR=\"${CMAKE_CURRENT_BINARY_DIR}\"
                Vidre_CURRENT_SOURCE_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}\"
        LINK_LIBRARIES 
            PRIVATE 
                Vidre::Core
)

if(FFmpeg_FOUND)
    vidre_test(decode_video EXCLUDE
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
endif()

if(Vidre_HAVE_MIDLIB2)
    vidre_test(aptina EXCLUDE
        BUILD
            COMPILE_OPTIONS
                PRIVATE
                    -Wno-pragma-pack
            LINK_LIBRARIES 
                PUBLIC
                    Vidre::Device::MIDlib2
                    /opt/Aptina_Imaging/lib/libapbase.so
        SOURCES aptina.cc
    )
endif()
