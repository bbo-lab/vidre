if(NOT APTINA_DIR)
    set(APTINA_DIR "C:/Aptina Imaging" CACHE PATH)
endif()

set(_LIB_NAME "apbase")

find_path(APTINA_INCLUDE_DIR apbase.h PATHS "${APTINA_DIR}/include")

find_library(APTINA_LIBRARIES NAMES ${_LIB_NAME} PATHS "${APTINA_DIR}/lib")

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(APTINA DEFAULT_MSG APTINA_INCLUDE_DIR APTINA_LIBRARIES)