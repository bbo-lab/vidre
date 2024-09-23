vidre_library(mydependency
    SOURCES 
        mydependency.cc
    BUILD
        LINK_LIBRARIES
            Vidre::Base
)

vidre_module(mymodule
    SOURCES 
        mymodule.cc
    BUILD
        COMPILE_DEFINITIONS
            PUBLIC
                "CMAKE_BUILD_DIR=\"${CMAKE_BUILD_DIR}\""
        LINK_LIBRARIES
            Vidre::Base
            mydependency
)

vidre_executable(myloader
    SOURCES
        myloader.cc
    BUILD        
        LINK_LIBRARIES
            Vidre::Base
)