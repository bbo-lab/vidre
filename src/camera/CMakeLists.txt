target_sources(vidre PRIVATE
    camera.hpp
    # dynamic_load.hpp
)

add_subdirectory(pylon)
add_subdirectory(aptina)
