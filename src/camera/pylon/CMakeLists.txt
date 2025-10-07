if(PYLON_ENABLED)
    add_library(vidre_pylon SHARED)
    add_library(vidre::pylon ALIAS vidre_pylon)

    target_sources(vidre_pylon PRIVATE
        pylon_camera.cpp pylon_camera.hpp
        pylon_buffer.hpp
    )

    target_include_directories(vidre_pylon PRIVATE 
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${pylon_INCLUDE_DIRS} 
    )

    target_link_libraries(vidre_pylon PRIVATE 
        vidre
        pylon::pylon
    )

    install(TARGETS vidre_pylon
        LIBRARY DESTINATION lib/vidre
        RUNTIME DESTINATION bin/vidre
    )
endif()