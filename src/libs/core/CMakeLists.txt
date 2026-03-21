vidre_library(vidrecore SHARED
    ALIAS Vidre::Core
    EXPORT VidreTargets-${Vidre_RELEASE}
    VERSION ${Vidre_VERSION}
    RELEASE ${Vidre_RELEASE}
    BUILD
        COMPILE_DEFINITIONS
            PRIVATE 
                VIDRE_CORE_VISIBILITY=VIDRE_API_EXPORT
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
            PRIVATE
                Boost::exception
                Boost::system
            PUBLIC
                DeLua::Library::CXX
                ${UUIDLIB}
                ${UNITSLIB}
                # ${TERMINALLIB}
                ${FMTLIB}
    INSTALL 
        LIBRARY COMPONENT runtime
        ARCHIVE COMPONENT development
        PUBLIC_HEADER COMPONENT development        
    SOURCES
        ${vidre_config_hh}
        vidre/core/config.hh vidre/core/config.cc
        vidre/os/tty.hh
        vidre/cpp/format.hh
        vidre/cpp/util.hh
        vidre/exception.hh vidre/exception.cc
        vidre/log.hh vidre/log.cc
        vidre/script.hh vidre/script.cc
        vidre/settings.hh vidre/settings.cc
        vidre/storage.hh
        vidre/plugin.hh 
            vidre/plugin/exception.hh
            vidre/plugin/information.hh
            vidre/plugin/dynamic.hh vidre/plugin/dynamic.cc
            vidre/plugin/hub.hh vidre/plugin/hub.cc
        vidre/util/version.hh
        vidre/util/develop.hh vidre/util/develop.cc
        vidre/util/uuid.hh vidre/util/uuid.cc
        vidre/util/sysinfo.hh vidre/util/sysinfo.cc
        vidre/util/json.hh
)
