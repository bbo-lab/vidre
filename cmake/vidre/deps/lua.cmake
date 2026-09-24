# Package: delua
# Version: 5.4 
#
# Info: This package is always fetched.

message(STATUS "Fetching jsawbbo/delua...")

FetchContent_Declare(delua
  GIT_REPOSITORY https://github.com/jsawbbo/delua.git
  GIT_TAG 5.4
  GIT_SHALLOW TRUE
  GIT_PROGRESS ON
)

set(LUA_BUILD_PAM OFF CACHE INTERNAL "")
set(LUA_BUILD_INTERPRETER OFF CACHE INTERNAL "")
set(LUA_BUILD_COMPILER OFF CACHE INTERNAL "")
set(LUA_BUILD_SHARED ON CACHE INTERNAL "")
set(LUA_BUILD_STATIC OFF CACHE INTERNAL "")
set(LUA_BUILD_PAM OFF CACHE INTERNAL "")

set(LUA_IDSIZE 256 CACHE INTERNAL "")

FetchContent_MakeAvailable(delua)

vidre_deps_set(delua ON) # always fetched
