vidre_test(GROUP 
    NAME core
    BUILD
        LINK_LIBRARIES 
            PRIVATE 
                Catch2::Catch2WithMain
                Vidre::Core
)

vidre_test(exceptions SOURCES exceptions.cc)
vidre_test(util SOURCES util.cc)
vidre_test(plugin SOURCES plugin.cc)
vidre_test(program SOURCES program.cc)
vidre_test(storage SOURCES storage.cc)
vidre_test(video SOURCES video.cc)

vidre_module(null 
    SOURCES plugin/null.cc
    PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib/test"
    BUILD
        LINK_LIBRARIES
            PUBLIC
                Vidre::Core
)
