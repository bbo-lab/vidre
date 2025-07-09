include(FetchContent)

FetchContent_Declare(lua
  GIT_REPOSITORY https://github.com/jsawbbo/delua.git
  GIT_TAG 5.4
  GIT_PROGRESS ON
)
FetchContent_MakeAvailable(lua)
