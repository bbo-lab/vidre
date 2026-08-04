set(CCV_HEADER_VERSION_MAJOR 0)
set(CCV_HEADER_VERSION_MINOR 12)

configure_file("${CMAKE_SOURCE_DIR}/include/vidre/config.h.in" "${CMAKE_BINARY_DIR}/config.h")

set(public_headers
    ${CMAKE_BINARY_DIR}/config.h
)

source_group(public FILES ${public_headers})

add_library(vidre STATIC ${public_headers})
# add_library(vidre::pylon STATIC ${public_headers})
# add_library(vidre::aptina STATIC ${public_headers})

target_include_directories(vidre PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(vidre PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_include_directories(vidre PUBLIC ${CMAKE_BINARY_DIR})



target_link_libraries(vidre PUBLIC Boost::headers)
target_link_libraries(vidre PUBLIC Boost::program_options)
target_link_libraries(vidre PUBLIC Boost::filesystem)
find_package(yaml-cpp REQUIRED)
target_link_libraries(vidre PUBLIC yaml-cpp::yaml-cpp)







add_subdirectory(core)
add_subdirectory(utils)
add_subdirectory(camera)

install(FILES ${public_headers} DESTINATION include/vidre)

install(TARGETS vidre
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)