vidre_test(GROUP 
    NAME core
    BUILD
        COMPILE_DEFINITIONS
            PRIVATE
                Vidre_CURRENT_BINARY_DIR=\"${CMAKE_CURRENT_BINARY_DIR}\"
                Vidre_CURRENT_SOURCE_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}\"
        LINK_LIBRARIES 
            PRIVATE 
                Vidre::Core
)

vidre_test(uuid SOURCES uuid.cc)
vidre_test(log SOURCES log.cc)
vidre_test(develop SOURCES develop.cc)
vidre_test(plugins SOURCES plugins.cc)
vidre_test(sysinfo SOURCES sysinfo.cc)
vidre_test(script SOURCES script.cc)
vidre_test(settings SOURCES settings.cc)

