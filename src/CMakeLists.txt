include(config.cmake)

set(vidre_build_include_dir ${PROJECT_BINARY_DIR}/include)
set(uoe_config_hh ${vidre_build_include_dir}/vidre/config.hh)

configure_file(config.hh.in ${uoe_config_hh})
set_source_files_properties(${uoe_config_hh}
    PROPERTIES
        GENERATED TRUE)

add_subdirectory(base)
# ...

# install(TARGETS FIXME)

