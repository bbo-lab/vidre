if(APTINA_ENABLED)
    add_library(vidre_aptina SHARED)
    add_library(vidre::aptina ALIAS vidre_aptina)

    target_sources(vidre_aptina PRIVATE
        aptina_camera.cpp aptina_camera.hpp
        aptina_buffer.hpp
    )

    target_include_directories(vidre_aptina PRIVATE 
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${APTINA_INCLUDE_DIR} 
    )

    target_link_libraries(vidre_aptina PRIVATE 
        vidre
        ${APTINA_LIBRARIES}
    )

    install(TARGETS vidre_aptina
        LIBRARY DESTINATION lib/vidre
        RUNTIME DESTINATION bin/vidre
    )
endif()