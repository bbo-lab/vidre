configure_file(config.hh.in ${vidre_config_hh})
set_source_files_properties(${vidre_config_hh}
    PROPERTIES
        GENERATED TRUE)

# for documentation purposes only:
if(config.hh.in IS_NEWER_THAN config.hh)
    file(READ config.hh.in FILE_CONTENTS)
    string(REGEX REPLACE "@[a-zA-Z0-9-_]*@" "<…>" FILE_CONTENTS ${FILE_CONTENTS})
    string(REGEX REPLACE "#cmakedefine01[ ]+([A-Za-z0-9_]+)" "#define \\1 <0/1>" FILE_CONTENTS ${FILE_CONTENTS})
    file(WRITE config.hh ${FILE_CONTENTS})
endif()
