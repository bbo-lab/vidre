# SOURCES #####################################################################
set(TARGET vidrebase)
set(ALIAS Vidre::Base)
set(SOURCES
    ${uoe_config_hh}
    vidre/base/config.hh vidre/base/config.cc
)

# OPTIONS #####################################################################
set(TARGET_LINK_LIBRARIES
    PUBLIC 
        Threads::Threads
)
set(TARGET_COMPILE_OPTIONS PRIVATE)
set(TARGET_COMPILE_DEFINITIONS
    PRIVATE 
        VIDRE_BASE_VISIBILITY=VIDRE_API_EXPORT
)
set(TARGET_COMPILE_FEATURES
    PUBLIC 
        cxx_std_23
)
set(TARGET_LINK_OPTIONS PRIVATE)
set(TARGET_INCLUDE_DIRECTORIES
    PUBLIC
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_INCLUDEDIR}>
    PRIVATE
        $<BUILD_INTERFACE:${vidre_build_include_dir}>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)
set(TARGET_INSTALL_OPTIONS 
    CONFIGURATIONS RelWithDebInfo
    LIBRARY
        COMPONENT development
    ARCHIVE
        COMPONENT development
    PUBLIC_HEADER
        COMPONENT development
)

# SHARED ######################################################################
add_library(${TARGET} SHARED
    ${SOURCES}
)
add_library(${ALIAS} ALIAS ${TARGET})
set_target_properties(${TARGET}
    PROPERTIES
        OUTPUT_NAME     ${TARGET}
        VERSION         ${vidre_VERSION}
        SOVERSION       ${vidre_VERSION_MAJOR}.${vidre_VERSION_MINOR}
)

target_link_libraries(${TARGET} ${TARGET_LINK_LIBRARIES})
target_compile_options(${TARGET} ${TARGET_COMPILE_OPTIONS})
target_compile_definitions(${TARGET} ${TARGET_COMPILE_DEFINITIONS})
target_compile_features(${TARGET} ${TARGET_COMPILE_FEATURES})
target_link_options(${TARGET} ${TARGET_LINK_OPTIONS})
target_include_directories(${TARGET} ${TARGET_INCLUDE_DIRECTORIES})
install(TARGETS ${TARGET} ${TARGET_INSTALL_OPTIONS})

# STATIC ######################################################################
add_library(${TARGET}-static STATIC
    ${SOURCES}
)
add_library(${ALIAS}::Static ALIAS ${TARGET}-static)
set_target_properties(${TARGET}-static
    PROPERTIES
        OUTPUT_NAME     ${TARGET}
        VERSION         ${vidre_VERSION}
        SOVERSION       ${vidre_VERSION_MAJOR}.${vidre_VERSION_MINOR}
)

target_link_libraries(${TARGET}-static ${TARGET_LINK_LIBRARIES})
target_compile_options(${TARGET}-static ${TARGET_COMPILE_OPTIONS})
target_compile_definitions(${TARGET}-static ${TARGET_COMPILE_DEFINITIONS})
target_compile_features(${TARGET}-static ${TARGET_COMPILE_FEATURES})
target_link_options(${TARGET}-static ${TARGET_LINK_OPTIONS})
target_include_directories(${TARGET}-static ${TARGET_INCLUDE_DIRECTORIES})
install(TARGETS ${TARGET}-static ${TARGET_INSTALL_OPTIONS})
