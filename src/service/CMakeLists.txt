find_package(cli11 CONFIG REQUIRED)

add_executable(vidre_camera_service camera_service_main.cpp)
target_link_libraries(vidre_camera_service PRIVATE vidre cli11::cli11)

add_executable(vidre_storage_service storage_service_main.cpp)
target_link_libraries(vidre_storage_service PRIVATE vidre cli11::cli11)

install(TARGETS vidre_camera_service vidre_storage_service
    RUNTIME DESTINATION bin
)
