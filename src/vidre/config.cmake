set(vidre_build_include_dir ${PROJECT_BINARY_DIR}/include)
set(vidre_config_hh ${vidre_build_include_dir}/vidre/config.hh)

# API
set(vidre_API_IMPORT "" CACHE STRING "Symbol import visibility.") # TODO
set(vidre_API_EXPORT "" CACHE STRING "Symbol export visibility.") # TODO
set(vidre_API_MODULE "VIDRE_API_EXPORT extern \"C\"" CACHE STRING "Module symbol visibility.") # TODO
