set(vidre_build_include_dir ${PROJECT_BINARY_DIR}/include)
set(vidre_config_hh ${vidre_build_include_dir}/vidre/config.hh)

# API
set(Vidre_API_IMPORT "" CACHE STRING "Symbol import visibility.") # TODO
set(Vidre_API_EXPORT "" CACHE STRING "Symbol export visibility.") # TODO
set(Vidre_API_MODULE "extern \"C\" VIDRE_API_EXPORT" CACHE STRING "Module symbol visibility.") # TODO
set(Vidre_DLL_EXTENSION ".so" CACHE STRING "File extension for dynamically linked libraries.") # TODO

