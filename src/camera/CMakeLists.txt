target_sources(vidre PRIVATE
    camera.hpp
    remote_camera.cpp remote_camera.hpp
)

add_subdirectory(dummy)
add_subdirectory(pylon)
add_subdirectory(aptina)
