vidre_library(vidrecore SHARED
    ALIAS Vidre::Core
    EXPORT VidreTargets-${Vidre_RELEASE}
    VERSION ${Vidre_VERSION}
    RELEASE ${Vidre_RELEASE}
    BUILD
        COMPILE_DEFINITIONS
            PRIVATE 
                VIDRE_CORE_VISIBILITY=VIDRE_API_EXPORT
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
                ${UUIDLIB}
    INSTALL 
        LIBRARY COMPONENT runtime
        ARCHIVE COMPONENT development
        PUBLIC_HEADER COMPONENT development        
    SOURCES
        ${vidre_config_hh}
        vidre/core/config.hh vidre/core/config.cc
        vidre/util/uuid.hh vidre/util/uuid.cc
        vidre/plugin.hh 
            vidre/plugin/information.hh
            vidre/plugin/dynamic.hh vidre/plugin/dynamic.cc
)
