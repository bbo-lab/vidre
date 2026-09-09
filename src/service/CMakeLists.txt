find_package(CLI11 CONFIG REQUIRED)

add_executable(vidre_camera_service camera_service_main.cpp)
target_link_libraries(vidre_camera_service PRIVATE vidre CLI11::CLI11)

add_executable(vidre_storage_service storage_service_main.cpp)
target_link_libraries(vidre_storage_service PRIVATE vidre CLI11::CLI11)

install(TARGETS vidre_camera_service vidre_storage_service
    RUNTIME DESTINATION bin
)
