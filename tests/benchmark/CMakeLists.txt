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

vidre_test(chrono EXCLUDE
    SOURCES chrono.cc
    BUILD
        LINK_LIBRARIES 
            PRIVATE 
                Vidre::Core
)

vidre_test(modulo EXCLUDE
    SOURCES modulo.cc
    BUILD
        LINK_LIBRARIES 
            PRIVATE 
                Vidre::Core
)

vidre_test(packetio EXCLUDE
    SOURCES 
        packetio.cc
        packetio/zero.hh packetio/zero.cc
    BUILD
        LINK_LIBRARIES 
            PRIVATE 
                Vidre::Core
)

