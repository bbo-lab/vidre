vidre_module(mymodule
    SOURCES 
        mymodule.cc
    BUILD
        INCLUDE_DIRECTORIES
            PRIVATE
                ${PROJECT_BINARY_DIR}/include 
                ${PROJECT_SOURCE_DIR}/src/libs/base
        LINK_LIBRARIES
            Vidre::Base
)

