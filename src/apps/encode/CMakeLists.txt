vidre_executable(vidre-encode
    ALIAS Vidre::App::Encode
    BUILD
        LINK_LIBRARIES
            PUBLIC Vidre::Core
    SOURCES
        main.cc
)
