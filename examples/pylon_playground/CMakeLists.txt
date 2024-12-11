cmake_minimum_required(VERSION 3.25)

project(PylonTest)

find_package(Pylon 8.0.0 REQUIRED)
find_package(Boost 1.86 REQUIRED COMPONENTS ALL)

add_executable(PylonTest
    main.cpp
    include/ConfigurationEventPrinter.h
    include/ImageEventPrinter.h
    pylon_camera.hpp
    pylon_camera.cpp
    pylon_shr_mem_buffer_factory.hpp
    shared_image_header.hpp

    # shared_image_memory_controller.hpp
)
target_link_libraries(PylonTest PRIVATE pylon::pylon Boost::headers)
