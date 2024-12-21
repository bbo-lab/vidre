configure_file("${CMAKE_SOURCE_DIR}/include/vidre/config.h.in" "${CMAKE_BINARY_DIR}/config.h")

set(public_headers
    ${CMAKE_BINARY_DIR}/config.h
    ../include/vidre/ring_buffer.h
    ../include/vidre/storage.h
    ../include/vidre/runtime.h
    ../include/vidre/pylon_camera.hpp
)

source_group(public FILES ${public_headers})

add_library(vidre SHARED ${public_headers})

target_include_directories(vidre PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(vidre PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_include_directories(vidre PUBLIC ${CMAKE_BINARY_DIR})

if(PYLON_ENABLED)
    message(STATUS "Using pylon SDK (Basler AG) 2222")
    target_link_libraries(vidre PRIVATE pylon::pylon)
endif()

add_subdirectory(ring_buffer)
add_subdirectory(runtime)
add_subdirectory(storage)
add_subdirectory(camera)

install(FILES ${public_headers} DESTINATION include/vidre)

install(TARGETS vidre
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)