# External software dependencies:
#
# There's a long list of dependencies. To make life easier for "simple" users
# we generate a report first if dependencies are unmet - which is also more
# developer friendly in terms of book keeping.
#
set(VIDRE_DEPS 
    boost
    ffmpeg
    json
    libfmt
    libuuid
    libyuv
    lua
    # units
    yaml
    unordered_dense
    ftxui
    qt

    catch2
)

# Dependency report.
set(VIDRE_DEPS_REPORT "" CACHE INTERNAL "Dependency report.")
set(VIDRE_DEPS_UNMET OFF CACHE INTERNAL "Unmet dependencies.")
function(vidre_deps_report)
    foreach(line ${ARGV})
        set(VIDRE_DEPS_REPORT "${VIDRE_DEPS_REPORT};${line}" CACHE INTERNAL "Dependency report.")
    endforeach()
endfunction()

function(vidre_deps_report_show)
    foreach(line ${VIDRE_DEPS_REPORT})
        message(NOTICE "${line}")
    endforeach()
endfunction()

function(vidre_deps_set PACKAGE FOUND)
    cmake_parse_arguments(_ "OPTIONAL;RECOMMENDED;REQUIRED" ${ARGV})

    if(FOUND)
        set(msg "  [x] ${PACKAGE}")
    else()
        if(__REQUIRED)
            set(msg "  [+] ${PACKAGE} (required)")
            set(VIDRE_DEPS_UNMET ON CACHE INTERNAL "Unmet dependencies.")
        elseif(__RECOMMENDED)
            set(msg "  [-] ${PACKAGE} (recommended)")
        else()
            set(msg "  [ ] ${PACKAGE}")
        endif()
    endif()
    vidre_deps_report(${msg})
endfunction()

vidre_deps_report("Dependency summary:")

# CMake configuration dependencies
include(VidreFetchContent)

find_package(PkgConfig QUIET)
vidre_deps_set(pkg-config ${PkgConfig_FOUND} RECOMMENDED)

# Check dependencies
set(VIDRE_DEPS_CHECK_ONLY ON)

foreach(dep ${VIDRE_DEPS})
    include(vidre/deps/${dep})
endforeach()

vidre_deps_report_show()

if(VIDRE_FETCH_DEPS)
    set(VIDRE_DEPS_CHECK_ONLY OFF)
    foreach(dep ${VIDRE_DEPS})
        include(vidre/deps/${dep})
    endforeach()
elseif(VIDRE_DEPS_UNMET)
    message(FATAL_ERROR "Unmet depencies, see https://github.com/bbo-lab/vidre/blob/main/cmake/DEPENDENCIES.md for further information.")
endif()
