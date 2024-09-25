vidre_test(GROUP 
    NAME core
    BUILD
        LINK_LIBRARIES 
            PRIVATE 
                Catch2::Catch2WithMain
                Vidre::Core
)

vidre_test(exceptions SOURCES exceptions.cc)
vidre_test(uuid SOURCES uuid.cc)