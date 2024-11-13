vidre_executable(vidre-encode
    ALIAS Vidre::App::Encode
    EXPORT VidreTargets-${Vidre_RELEASE}
    PROPERTIES
        AUTOMOC ON
    BUILD
        LINK_LIBRARIES
            PUBLIC 
                Vidre::Core
                Vidre::FFmpeg
                Qt6::Core
        INCLUDE_DIRECTORIES
            PUBLIC
                $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    SOURCES
        main.cc
)

# message("QT_INCLUDES=${QT_INCLUDES}")