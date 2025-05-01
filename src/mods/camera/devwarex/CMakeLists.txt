find_library(MIDLIB2_LIB
    NAMES midlib2
    HINTS
        "/opt/Aptina_Imaging/lib"
        "C:/Aptina_Imaging/lib"
)
if(MIDLIB2_LIB)
    find_path(MIDLIB2_INC 
        NAMES midlib2.h
        HINTS
            "/opt/Aptina_Imaging/include"
            "C:/Aptina_Imaging/include"
    )

    if (MIDLIB2_INC)
        add_library(midlib2 UNKNOWN IMPORTED GLOBAL)
        add_library(devwarex::midlib2 ALIAS midlib2)
        target_include_directories(midlib2
            INTERFACE ${MIDLIB2_INC})

        message(STATUS "Using midlib2 (Micron Imaging Device LIBrary, DevWareX SDK, OnSemi).")
    endif()
endif()

