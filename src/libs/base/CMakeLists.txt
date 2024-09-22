vidre_library(vidrebase SHARED
    ALIAS Vidre::Base 
    EXPORT VidreTargets-${Vidre_RELEASE}
    VERSION ${Vidre_VERSION}
    RELEASE ${Vidre_RELEASE}
    BUILD
        COMPILE_DEFINITIONS
            PRIVATE 
                VIDRE_BASE_VISIBILITY=VIDRE_API_EXPORT
        COMPILE_FEATURES
            PUBLIC 
                cxx_std_23
        INCLUDE_DIRECTORIES
            PUBLIC
                $<INSTALL_INTERFACE:${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_INCLUDEDIR}>
            PRIVATE
                $<BUILD_INTERFACE:${vidre_build_include_dir}>
                $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    INSTALL 
        LIBRARY COMPONENT runtime
        ARCHIVE COMPONENT development
        PUBLIC_HEADER COMPONENT development        
    SOURCES
        ${vidre_config_hh}
        vidre/base.cc
        vidre/base/config.hh vidre/base/config.cc
        vidre/util/uuid.hh vidre/util/uuid.cc
        vidre/module.hh 
            vidre/module/declare.hh
            vidre/module/module.cc
)
