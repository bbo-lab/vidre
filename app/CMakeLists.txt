cmake_minimum_required(VERSION 3.25)

# Qt6 setup
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

# Boost
find_package(Boost REQUIRED COMPONENTS program_options filesystem)

# YAML
# find_package(yaml-cpp REQUIRED)

add_executable(vidre_app
    main.cpp
    mainwindow.cpp
    mainwindow.hpp
    mainwindow.ui
    camerawidget.cpp
    camerawidget.hpp
    camerawidget.ui
    devicelistdock.cpp
    devicelistdock.hpp
)

target_link_libraries(vidre_app PRIVATE
    Qt6::Core
    Qt6::Widgets
    vidre
    Boost::program_options
    Boost::filesystem
    # yaml-cpp
)

target_include_directories(vidre_app PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../src
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

install(TARGETS vidre_app RUNTIME DESTINATION bin)
