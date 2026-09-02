add_executable(highload_save_test
    highload_save_test.cpp
)

target_link_libraries(highload_save_test PRIVATE
    vidre
    Boost::program_options
)

target_include_directories(highload_save_test PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/include
)

add_test(NAME highload_save_test COMMAND highload_save_test)

function(vidre_add_unit_test name)
    add_executable(${name} ${name}.cpp)
    target_link_libraries(${name} PRIVATE vidre)
    target_include_directories(${name} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    add_test(NAME ${name} COMMAND ${name})
    set_tests_properties(${name} PROPERTIES TIMEOUT 60)
endfunction()

vidre_add_unit_test(wire_codec_test)
vidre_add_unit_test(buffer_ring_test)
vidre_add_unit_test(config_loader_test)
vidre_add_unit_test(ccv_roundtrip_test)
vidre_add_unit_test(storage_server_session_test)
vidre_add_unit_test(remote_sink_test)

function(vidre_add_e2e_test name)
    add_executable(${name} ${name}.cpp)
    target_link_libraries(${name} PRIVATE vidre Boost::system)
    target_include_directories(${name} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    add_test(NAME ${name}
        COMMAND ${name}
            $<TARGET_FILE:vidre_camera_service>
            $<TARGET_FILE:vidre_storage_service>)
    set_tests_properties(${name} PROPERTIES TIMEOUT 60)
endfunction()

vidre_add_e2e_test(camera_service_e2e_test)
vidre_add_e2e_test(distributed_pipeline_test)