vidre_test(DEFAULTS 
    NAME benchmark)

vidre_test(exceptions EXCLUDE
    SOURCES exceptions.cc
    BUILD
        LINK_LIBRARIES 
            PRIVATE 
                Vidre::Core
)

vidre_test(raw EXCLUDE
    SOURCES raw.cc
    BUILD
        LINK_LIBRARIES 
            PRIVATE 
                Vidre::Core
)
