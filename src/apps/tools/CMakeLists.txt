vidre_executable(vidre-uuid
    ALIAS Vidre::App::UUID
    EXPORT VidreTargets-${Vidre_RELEASE}
    BUILD
        LINK_LIBRARIES
            PUBLIC 
                Vidre::Core
        INCLUDE_DIRECTORIES
            PUBLIC
                $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    SOURCES
        uuid.cc
)

