cmake_minimum_required(VERSION 3.25)

# Qt6 setup

# find_package(Qt6 REQUIRED COMPONENTS Core Widgets Gui)
# set(CMAKE_FIND_DEBUG_MODE 1)
find_package(Qt6 COMPONENTS Core Gui Widgets REQUIRED)
# set(CMAKE_FIND_DEBUG_MODE 0)
if(NOT Qt6_FOUND)
message(STATUS "!!!!Qt6 not found!!!")
endif()
message(STATUS "${Qt6_LIBRARIES}")
message(STATUS "${Qt6_INCLUDE_DIRS}")
# message(STATUS "!!!!Qt6 not found!!!")

qt_standard_project_setup()
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
# Boost
find_package(Boost REQUIRED COMPONENTS program_options filesystem)
# YAML
find_package(yaml-cpp REQUIRED)



set(SOURCES
    main.cpp
    mainwindow.cpp
    mainwindow.hpp
    camerawidget.cpp
    camerawidget.hpp
    camerainfosdock.cpp
    camerainfosdock.hpp
    devicelistdock.cpp
    devicelistdock.hpp
    status_mapping.hpp
    camerawidget.ui
    mainwindow.ui
)

# qt_wrap_ui(SOURCES
#     camerawidget.ui
#     mainwindow.ui
# )
add_executable(vidre_app ${SOURCES})
target_link_libraries(vidre_app PUBLIC
    Qt6::Core
    Qt6::Widgets
    Qt6::Gui
    vidre
    Boost::program_options
    Boost::filesystem
    yaml-cpp::yaml-cpp
)
set_target_properties(vidre_app PROPERTIES
    WIN32_EXECUTABLE ON
)
target_include_directories(vidre_app PUBLIC
        "${CMAKE_CURRENT_BINARY_DIR}/")
target_include_directories(vidre_app PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../src
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

install(TARGETS vidre_app 
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION bin
)

# Generate a deployment script to automatically pull in Qt libraries and plugins
qt_generate_deploy_app_script(
    TARGET vidre_app
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)

install(SCRIPT ${deploy_script})

if(MSVC)
    target_compile_options(vidre_app PRIVATE /W4 /WX- /bigobj)
endif()