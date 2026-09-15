set(vidre_build_include_dir ${PROJECT_BINARY_DIR}/include)
set(vidre_config_hh ${vidre_build_include_dir}/vidre/config.hh)

# API
set(VIDRE_API_IMPORT "" CACHE STRING "Symbol import visibility.") # TODO
set(VIDRE_API_EXPORT "" CACHE STRING "Symbol export visibility.") # TODO
set(VIDRE_API_MODULE "extern \"C\" VIDRE_API_EXPORT" CACHE STRING "Module symbol visibility.") # TODO
mark_as_advanced(VIDRE_API_IMPORT VIDRE_API_EXPORT VIDRE_API_MODULE)

set(VIDRE_DLL_EXTENSION ".so" CACHE STRING "File extension for dynamically linked libraries.") # TODO
mark_as_advanced(VIDRE_DLL_EXTENSION)

# Storage
set(VIDRE_CACHE_SIZE "1048576" CACHE STRING "Storage size for IO caches (default: 1 MiB).")
set(VIDRE_CACHE_SLOTS "16" CACHE STRING "Default number of slots in IO caches.")
set(VIDRE_CACHE_TIMEOUT_MS "100" CACHE STRING "Timeout for IO buffer operations in milliseconds.")
mark_as_advanced(VIDRE_CACHE_SIZE VIDRE_CACHE_SLOTS VIDRE_CACHE_TIMEOUT_MS)

