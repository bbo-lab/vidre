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
                unordered_dense::unordered_dense 
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
        vidre/cpp/shared.hh
        vidre/util.cc
            vidre/util/version.hh vidre/util/version.cc
            vidre/util/develop.hh vidre/util/develop.cc
            vidre/util/uuid.hh vidre/util/uuid.cc
            vidre/util/filesystem.hh vidre/util/filesystem.cc
            vidre/util/variant.hh
            vidre/util/endian.hh
            vidre/util/bit.hh vidre/util/bit.cc
            vidre/util/chrono.hh vidre/util/chrono.cc
        vidre/error.hh vidre/error.cc
            vidre/error/details.hh
            vidre/error/flow.hh
            vidre/error/exceptions.hh
        vidre/log.hh vidre/log.cc
        vidre/memory.hh vidre/memory.cc
            vidre/memory/base.hh
            vidre/memory/chunk.hh
        vidre/script.hh vidre/script.cc
        vidre/settings.hh vidre/settings.cc
            vidre/settings/lua.hh 
            vidre/settings/json.hh
            vidre/settings/yaml.hh 
        vidre/io.hh vidre/io.cc
            vidre/io/types.hh vidre/io/types.cc
                vidre/io/types/core.hh vidre/io/types/core.cc
                vidre/io/types/sink.hh vidre/io/types/sink.cc
                vidre/io/types/endpoint.hh vidre/io/types/endpoint.cc
                vidre/io/types/source.hh vidre/io/types/source.cc
            #     vidre/io/types/channel.hh vidre/io/types/channel.cc
            #     vidre/io/types/merger.hh vidre/io/types/merger.cc
            #     vidre/io/types/arbiter.hh vidre/io/types/arbiter.cc
            vidre/io/packet.hh vidre/io/packet.cc
                vidre/io/packet/base.hh vidre/io/packet/base.cc
                vidre/io/packet/factory.hh vidre/io/packet/factory.cc
                vidre/io/packet/value.hh vidre/io/packet/value.cc
                vidre/io/packet/sequence.hh vidre/io/packet/sequence.cc
                vidre/io/packet/object.hh vidre/io/packet/object.cc
            vidre/io/file.hh vidre/io/file.cc
            # vidre/io/monitor.hh vidre/io/monitor.cc
            # vidre/io/flow.hh vidre/io/flow.cc
        vidre/storage.hh vidre/storage.cc
            vidre/storage/raw.hh vidre/storage/raw.cc
        vidre/thread.hh vidre/thread.cc
            vidre/thread/types.hh 
            vidre/thread/task.hh vidre/thread/task.cc
            vidre/thread/schedule.hh vidre/thread/schedule.cc
        vidre/plugin.hh vidre/plugin.cc
            vidre/plugin/types.hh vidre/plugin/types.cc
            vidre/plugin/exception.hh
            vidre/plugin/information.hh
            vidre/plugin/native.hh vidre/plugin/native.cc
        vidre/program.hh vidre/program.cc        
)
