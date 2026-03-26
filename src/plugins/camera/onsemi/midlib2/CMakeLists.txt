find_library(MIDLIB2_LIB
    NAMES midlib2
    HINTS
        "/opt/Aptina_Imaging/lib"
        "C:/Aptina_Imaging/lib"
        "C:/Aptina Imaging/lib"
        "$ENV{DEVWAREX_ROOT}/lib"
        "$ENV{APTINA_ROOT}/lib"
)
if(MIDLIB2_LIB)
    find_path(MIDLIB2_INC 
        NAMES midlib2.h
        HINTS
            "/opt/Aptina_Imaging/include"
            "C:/Aptina_Imaging/include"
            "C:/Aptina Imaging/include"
            "$ENV{DEVWAREX_ROOT}/include"
            "$ENV{APTINA_ROOT}/include"
    )

    if (MIDLIB2_INC)
        add_library(midlib2 UNKNOWN IMPORTED GLOBAL)
        add_library(devwarex::midlib2 ALIAS midlib2)
        target_include_directories(midlib2
            INTERFACE ${MIDLIB2_INC})
        set_target_properties(midlib2 
            PROPERTIES 
                IMPORTED_LOCATION ${MIDLIB2_LIB})

        message(STATUS "Using midlib2 (Micron Imaging Device LIBrary, DevWareX SDK, OnSemi).")
        set(VIDRE_HAVE_MIDLIB2 TRUE CACHE BOOL "DevWareX midlib2 library and includes found")
    endif()
endif()
if(NOT VIDRE_HAVE_MIDLIB2)
    set(VIDRE_HAVE_MIDLIB2 FALSE CACHE BOOL "DevWareX midlib2 library and includes found")
endif()

if(VIDRE_HAVE_MIDLIB2)
    set(PLUGIN_CURRENT_DIR camera/onsemi/midlib2)

    vidre_library(vidre-midlib2 SHARED
        ALIAS Vidre::Device::MIDlib2
        EXPORT VidreTargets-${Vidre_RELEASE}
        VERSION ${Vidre_VERSION}
        RELEASE ${Vidre_RELEASE}
        BUILD
            COMPILE_FEATURES
                PUBLIC 
                    cxx_std_23
            COMPILE_DEFINITIONS
                PRIVATE 
                    VIDRE_MIDLIB2_VISIBILITY=VIDRE_API_EXPORT
            COMPILE_OPTIONS
                PRIVATE
                    -Wno-pragma-pack
            INCLUDE_DIRECTORIES
                PUBLIC
                    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
                    ${MIDLIB2_INC}
            LINK_LIBRARIES
                PRIVATE
                    Boost::exception
                    Boost::system
                PUBLIC
                    Vidre::Core
                    ${MIDLIB2_LIB}
        INSTALL 
            LIBRARY COMPONENT runtime
            ARCHIVE COMPONENT development
            PUBLIC_HEADER COMPONENT development        
        SOURCES
            vidre/onsemi/midlib2.hh vidre/onsemi/midlib2.cc
            vidre/onsemi/midlib2/config.hh
            vidre/onsemi/midlib2/types.hh
    )

    vidre_module(onsemi-midlib2
        PROPERTIES
            OUTPUT_NAME midlib2
            LIBRARY_OUTPUT_DIRECTORY ${PLUGIN_OUTPUT_DIR}/${PLUGIN_CURRENT_DIR}
        BUILD
            LINK_LIBRARIES
                PUBLIC 
                    Vidre::Core
                    devwarex::midlib2
        INSTALL
            DESTINATION ${CMAKE_INSTALL_PLUGINDIR}/${PLUGIN_CURRENT_DIR}
            COMPONENT "runtime"
        SOURCES
            midlib2.cc
    )

    vidre_module(onsemi-midlib2-check
        PROPERTIES
            OUTPUT_NAME check
            LIBRARY_OUTPUT_DIRECTORY ${PLUGIN_OUTPUT_DIR}/${PLUGIN_CURRENT_DIR}
        BUILD
            COMPILE_DEFINITIONS
                PRIVATE 
                    LIBMIDLIB2_NAME=\"libmilib2${vidre_DLL_EXTENSION}\"
            LINK_LIBRARIES
                PUBLIC 
                    Vidre::Core
        INSTALL
            DESTINATION "${CMAKE_INSTALL_PLUGINDIR}/${PLUGIN_CURRENT_DIR}"
            COMPONENT "runtime"
        SOURCES
            check.cc
    )

    configure_file(plugin.lua ${PLUGIN_OUTPUT_DIR}/${PLUGIN_CURRENT_DIR}/plugin.cfg COPYONLY)
    install(FILES plugin.lua
        RENAME plugin.cfg
        DESTINATION ${CMAKE_INSTALL_PLUGINDIR}/${PLUGIN_CURRENT_DIR}
        COMPONENT "runtime")
endif()
