include(FetchContent)

# SI-Units
set(MP_UNITS_API_CONTRACTS NONE)
# set(MP_UNITS_BUILD_CXX_MODULES ON)
FetchContent_Declare(mp-units
  GIT_REPOSITORY https://github.com/mpusz/mp-units
  GIT_TAG 2.2
  SOURCE_SUBDIR src
)
FetchContent_MakeAvailable(mp-units)
set(HAVE_UNITS_MP_UNITS 1)
set(UNITSLIB mp-units)

