# Package: mp-units
# Version: 2.2
#
# Info: This package is always fetched.

message(STATUS "Fetching mpusz/mp-units...")

FetchContent_Declare(mp-units
  GIT_REPOSITORY https://github.com/mpusz/mp-units
  GIT_TAG        2.2
  GIT_SHALLOW    TRUE
  GIT_PROGRESS   ON
  SOURCE_SUBDIR  src
)

set(MP_UNITS_API_CONTRACTS NONE)
set(MP_UNITS_API_NO_CRTP OFF)
set(MP_UNITS_BUILD_CXX_MODULES ON)

FetchContent_MakeAvailable(mp-units)
set(HAVE_UNITS_MP_UNITS 1)
set(UNITSLIB mp-units)

vidre_deps_set(mp-units ON) # always fetched
