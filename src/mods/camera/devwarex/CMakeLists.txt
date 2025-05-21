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
    vidre_module(devwarex-module
        PROPERTIES
            OUTPUT_NAME module
            LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/module/onsemi/devwarex
        BUILD
            LINK_LIBRARIES
                PUBLIC 
                    Vidre::Core
                    devwarex::midlib2
        SOURCES
            module.cc
    )
endif()
