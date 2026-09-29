vidre_test(GROUP 
    NAME core
    BUILD
        COMPILE_DEFINITIONS
            PRIVATE
                Vidre_CURRENT_BINARY_DIR=\"${CMAKE_CURRENT_BINARY_DIR}\"
                Vidre_CURRENT_SOURCE_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}\"
        INCLUDE_DIRECTORIES
            PRIVATE
                ${CMAKE_SOURCE_DIR}/tests
        LINK_LIBRARIES 
            PRIVATE 
                Vidre::Core
)

vidre_test(uuid SOURCES uuid.cc)
vidre_test(task SOURCES task.cc)
vidre_test(log SOURCES log.cc)
vidre_test(develop SOURCES develop.cc)
vidre_test(os SOURCES os.cc)
vidre_test(script SOURCES script.cc)
# vidre_test(settings SOURCES settings.cc)
vidre_test(io SOURCES io.cc)
vidre_test(cache SOURCES cache.cc)
vidre_test(packet SOURCES packet.cc)
vidre_test(iograph SOURCES iograph.cc)

# vidre_test(program EXCLUDE SOURCES program.cc)
# vidre_test(raw EXCLUDE SOURCES raw.cc)
vidre_test(event EXCLUDE SOURCES event.cc)

