include(FetchContent)

FetchContent_Declare(lua
  GIT_REPOSITORY https://github.com/jsawbbo/delua.git
  GIT_BRANCH 5.4
)
FetchContent_MakeAvailable(lua)
