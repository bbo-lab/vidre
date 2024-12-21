add_executable(grab_and_save main.cpp)

target_link_libraries(grab_and_save PRIVATE vidre)

install(TARGETS grab_and_save RUNTIME DESTINATION bin)
# set_property(TARGET grab_and_save PROPERTY FOLDER "tools")