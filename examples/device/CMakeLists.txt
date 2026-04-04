vidre_test(GROUP 
    NAME device
    BUILD
        COMPILE_DEFINITIONS
            PRIVATE
                Vidre_CURRENT_BINARY_DIR=\"${CMAKE_CURRENT_BINARY_DIR}\"
                Vidre_CURRENT_SOURCE_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}\"
        LINK_LIBRARIES 
            PRIVATE 
                Vidre::Device
)

# vidre_test(plugins SOURCES plugins.cc)

