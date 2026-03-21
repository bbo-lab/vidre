include(FetchContent)

FetchContent_Declare(lua
  GIT_REPOSITORY https://github.com/jsawbbo/delua.git
  GIT_TAG 5.4
  GIT_PROGRESS ON
)

set(LUA_BUILD_PAM OFF CACHE INTERNAL "")
set(LUA_BUILD_INTERPRETER OFF CACHE INTERNAL "")
set(LUA_BUILD_COMPILER OFF CACHE INTERNAL "")
set(LUA_BUILD_SHARED ON CACHE INTERNAL "")
set(LUA_BUILD_STATIC OFF CACHE INTERNAL "")

FetchContent_MakeAvailable(lua)
