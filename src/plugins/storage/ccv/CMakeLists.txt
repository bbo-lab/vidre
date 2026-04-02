set(PLUGIN_CURRENT_DIR camera/onsemi/midlib2)

vidre_library(vidre-store-ccv SHARED
    ALIAS Vidre::Storage::CCV
    EXPORT VidreTargets-${Vidre_RELEASE}
    VERSION ${Vidre_VERSION}
    RELEASE ${Vidre_RELEASE}
    BUILD
        COMPILE_FEATURES
            PUBLIC 
                cxx_std_23
        COMPILE_DEFINITIONS
            PRIVATE 
                VIDRE_CCV_VISIBILITY=VIDRE_API_EXPORT
        COMPILE_OPTIONS
        INCLUDE_DIRECTORIES
            PUBLIC
                $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        LINK_LIBRARIES
            PUBLIC
                Vidre::Core
                Boost::iostreams
    INSTALL 
        LIBRARY COMPONENT runtime
        ARCHIVE COMPONENT development
        PUBLIC_HEADER COMPONENT development        
    SOURCES
        vidre/storage/ccv.hh vidre/storage/ccv.cc
)

# vidre_module(onsemi-midlib2
#     PROPERTIES
#         OUTPUT_NAME midlib2
#         LIBRARY_OUTPUT_DIRECTORY ${PLUGIN_OUTPUT_PATH}/${PLUGIN_CURRENT_DIR}
#     BUILD
#         LINK_LIBRARIES
#             PUBLIC 
#                 Vidre::Core
#                 devwarex::midlib2
#     INSTALL
#         DESTINATION ${CMAKE_INSTALL_PLUGINDIR}/${PLUGIN_CURRENT_DIR}
#         COMPONENT "runtime"
#     SOURCES
#         midlib2.cc
# )

# configure_file(plugin.lua ${PLUGIN_OUTPUT_PATH}/${PLUGIN_CURRENT_DIR}/plugin.cfg COPYONLY)
# install(FILES plugin.lua
#     RENAME plugin.cfg
#     DESTINATION ${CMAKE_INSTALL_PLUGINDIR}/${PLUGIN_CURRENT_DIR}
#     COMPONENT "runtime")
