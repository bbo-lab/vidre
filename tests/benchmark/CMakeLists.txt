vidre_test(DEFAULTS 
    NAME benchmark)

vidre_test(exceptions EXCLUDE
    SOURCES exceptions.cc
    BUILD
        LINK_LIBRARIES 
            PRIVATE 
                Vidre::Core
)

vidre_test(lookup EXCLUDE
    SOURCES lookup.cc
    BUILD
        LINK_LIBRARIES 
            PRIVATE 
                Vidre::Core
                unordered_dense::unordered_dense
)

vidre_test(modulo EXCLUDE
    SOURCES modulo.cc
    BUILD
        LINK_LIBRARIES 
            PRIVATE 
                Vidre::Core
                unordered_dense::unordered_dense
)

