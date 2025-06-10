set(CCV_HEADER_VERSION_MAJOR 0)
set(CCV_HEADER_VERSION_MINOR 12)

configure_file("${CMAKE_SOURCE_DIR}/include/vidre/config.h.in" "${CMAKE_BINARY_DIR}/config.h")

set(public_headers
    ${CMAKE_BINARY_DIR}/config.h
    ../include/vidre/storage.hpp
    ../include/vidre/runtime.hpp
    ../include/vidre/pylon_camera.hpp
    ../include/vidre/dummy_camera.hpp
    ../include/vidre/shared_frame_buffer_controller.hpp
    ../include/vidre/shared_frame_header.hpp
    ../include/vidre/camera.hpp
    ../include/vidre/config_loader.hpp
    ../include/vidre/presenter.hpp
    ../include/vidre/header.hpp
    ../include/vidre/buffer.hpp
)

source_group(public FILES ${public_headers})

add_library(vidre STATIC ${public_headers})

target_include_directories(vidre PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(vidre PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_include_directories(vidre PUBLIC ${CMAKE_BINARY_DIR})

if(PYLON_ENABLED)
    message(STATUS "Using pylon SDK (Basler AG)")
    target_link_libraries(vidre PUBLIC pylon::pylon)
endif()

if(APTINA_ENABLED)
    target_include_directories(vidre PUBLIC ${APTINA_INCLUDE_DIR})
    target_link_libraries(vidre PUBLIC ${APTINA_LIBRARIES})
endif()

target_link_libraries(vidre PUBLIC Boost::headers)
target_link_libraries(vidre PUBLIC Boost::program_options)
target_link_libraries(vidre PUBLIC Boost::filesystem)

target_link_libraries(vidre PUBLIC yaml-cpp::yaml-cpp)

add_subdirectory(runtime)
add_subdirectory(storage)
add_subdirectory(camera)
add_subdirectory(config_loader)
add_subdirectory(header)
add_subdirectory(buffer)

install(FILES ${public_headers} DESTINATION include/vidre)

install(TARGETS vidre
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)