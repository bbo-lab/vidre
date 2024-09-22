# This is free and unencumbered CMake code released into the public domain.
#
# Anyone is free to copy, modify, publish, use, compile, sell, or
# distribute this software, either in source code form or as a compiled
# binary, for any purpose, commercial or non-commercial, and by any
# means.
#
# In jurisdictions that recognize copyright laws, the author or authors
# of this software dedicate any and all copyright interest in the
# software to the public domain. We make this dedication for the benefit
# of the public at large and to the detriment of our heirs and
# successors. We intend this dedication to be an overt act of
# relinquishment in perpetuity of all present and future rights to this
# software under copyright law.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
# IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
# OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
# ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
# OTHER DEALINGS IN THE SOFTWARE.
#
# For more information, please refer to <https://unlicense.org>

# TODO documentation

function(vidre_build_options TARGET)
    cmake_parse_arguments(vidre "" "" "COMPILE_DEFINITIONS;COMPILE_FEATURES;COMPILE_OPTIONS;LINK_LIBRARIES;LINK_OPTIONS;INCLUDE_DIRECTORIES" ${ARGN})
    if(vidre_COMPILE_DEFINITIONS)
        target_compile_definitions(${TARGET} ${vidre_COMPILE_DEFINITIONS})
    endif()
    if(vidre_COMPILE_FEATURES)
        target_compile_features(${TARGET} ${vidre_COMPILE_FEATURES})
    endif()
    if(vidre_COMPILE_OPTIONS)
        target_compile_options(${TARGET} ${vidre_COMPILE_OPTIONS})
    endif()
    if(vidre_LINK_LIBRARIES)
        target_link_libraries(${TARGET} ${vidre_LINK_LIBRARIES})
    endif()
    if(vidre_LINK_OPTIONS)
        target_link_options(${TARGET} ${vidre_LINK_OPTIONS})
    endif()
    if(vidre_INCLUDE_DIRECTORIES)
        target_include_directories(${TARGET} ${vidre_INCLUDE_DIRECTORIES})
    endif()
endfunction()

function(vidre_install TARGET)
    message("INSTALL: ${TARGET} ${ARGN}")
    install(TARGETS ${TARGET}
        ${EXPORT_OPTIONS}
        ${ARGN}
    )
endfunction()

function(vidre_library TARGET)
    cmake_parse_arguments(vidre "SHARED;STATIC" "EXPORT;ALIAS" "SOURCES;BUILD;INSTALL" ${ARGN})

    if(vidre_SHARED AND vidre_STATIC)
        set(OPTIONS ALIAS ${vidre_ALIAS} SOURCES ${vidre_SOURCES})
        if(vidre_BUILD)
            list(APPEND OPTIONS BUILD ${vidre_BUILD})
        endif()
        if(vidre_INSTALL)
            list(APPEND OPTIONS INSTALL ${vidre_INSTALL})
        endif()

        vidre_library(${TARGET} STATIC ${OPTIONS})
    elseif(NOT vidre_SHARED AND NOT vidre_STATIC)
        set(vidre_SHARED ON)
    endif()

    set(OUTPUT_NAME ${TARGET})

    if(vidre_SHARED)
        set(vidre_TARGET ${TARGET})
        set(vidre_TYPE SHARED)
    else()
        set(vidre_TARGET ${TARGET}-static)
        set(vidre_TYPE STATIC)

        if(vidre_ALIAS)
            set(vidre_ALIAS ${vidre_ALIAS}::Static)
        endif()
    endif()

    if(NOT vidre_VERSION)
        set(vidre_VERSION ${PROJECT_VERSION})
    endif()

    if(NOT vidre_RELEASE)
        set(vidre_RELEASE ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR})
    endif()

    if(vidre_EXPORT)
        if(vidre_ALIAS)
            set(vidre_EXPORT_NAME EXPORT_NAME ${vidre_ALIAS})
        else()
            set(vidre_EXPORT_NAME EXPORT_NAME ${vidre_TARGET})
        endif()
        set(vidre_EXPORT EXPORT ${vidre_EXPORT})
    endif()

    add_library(${vidre_TARGET} ${vidre_TYPE} ${vidre_SOURCES})
    set_target_properties(${vidre_TARGET}
        PROPERTIES
            OUTPUT_NAME     ${TARGET}
            ${vidre_EXPORT_NAME}
            VERSION         ${vidre_VERSION}
            SOVERSION       ${vidre_RELEASE}
    )
    if(vidre_ALIAS)
        add_library(${vidre_ALIAS} ALIAS ${vidre_TARGET})
    endif()
    if(vidre_BUILD)
        vidre_build_options(${vidre_TARGET} ${vidre_BUILD})
    endif()
    if(vidre_INSTALL)
        vidre_install(${vidre_TARGET} ${vidre_EXPORT} ${vidre_INSTALL})
    endif()
endfunction()
