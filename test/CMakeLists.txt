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