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
                ${FMTLIB}
                ${YAMLLIB}
                ${JSONLIB}
    INSTALL 
        LIBRARY COMPONENT runtime
        ARCHIVE COMPONENT development
        PUBLIC_HEADER COMPONENT development        
    SOURCES
        ${vidre_config_hh}
        vidre/core/config.hh vidre/core/config.cc
        vidre/os.hh
            vidre/os/tty.hh
            vidre/os/memory.hh vidre/os/memory.cc
            vidre/os/paths.hh vidre/os/paths.cc
        vidre/cpp/format.hh
        vidre/cpp/util.hh
        vidre/util/version.hh vidre/util/version.cc
        vidre/util/develop.hh vidre/util/develop.cc
        vidre/util/uuid.hh vidre/util/uuid.cc
        vidre/util/filesystem.hh vidre/util/filesystem.cc
        vidre/util/variant.hh
        vidre/exception.hh vidre/exception.cc
        vidre/log.hh vidre/log.cc
        vidre/script.hh vidre/script.cc
        vidre/settings.hh vidre/settings.cc
            vidre/settings/lua.hh 
            vidre/settings/json.hh
            vidre/settings/yaml.hh 
        vidre/storage.hh vidre/storage.cc
            vidre/storage/io.hh vidre/storage/io.cc
            vidre/storage/file.hh vidre/storage/file.cc
            vidre/storage/bit.hh vidre/storage/bit.cc
            vidre/storage/raw.hh vidre/storage/raw.cc
        vidre/video.hh vidre/video.cc
            # vidre/video/common.hh
            # vidre/video/format.hh vidre/video/format.cc
                # vidre/video/format/geometry.hh vidre/video/format/geometry.cc
                # vidre/video/format/pixel.hh vidre/video/format/pixel.cc
                # vidre/video/format/color.hh vidre/video/format/color.cc
            # vidre/video/frame.hh vidre/video/frame.cc
        vidre/plugin.hh vidre/plugin.cc
            vidre/plugin/exception.hh
            vidre/plugin/information.hh
            vidre/plugin/dynamic.hh vidre/plugin/dynamic.cc
        vidre/program.hh vidre/program.cc        
)
