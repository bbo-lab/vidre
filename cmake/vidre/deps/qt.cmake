# Package: Qt6
# Version: 6.10
# Components: Core Gui Network Widgets
#
# Ubuntu:
# - qt6-base-dev

if(VIDRE_DEPS_CHECK_ONLY)
    find_package(Qt6 
        6.10  
        COMPONENTS 
            Core 
            Gui 
            Network 
            Widgets
    )
elseif(NOT Qt6_FOUND)
    set(VIDRE_BUILD_GUI OFF CACHE BOOL "Build GUIs." FORCE)
endif()

if(Qt6_FOUND)
    set(CMAKE_AUTOMOC OFF)
    set(CMAKE_AUTORCC OFF)
    set(CMAKE_AUTOUIC OFF)
endif()
