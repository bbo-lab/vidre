# Package: cpp-terminal
# Version: 0.1
#
# Info: This package is always fetched.

FetchContent_Declare(cpp-terminal
  GIT_REPOSITORY https://github.com/jupyter-xeus/cpp-terminal.git
  GIT_SHALLOW    TRUE
  GIT_PROGRESS   ON
)

set(CPPTERMINAL_BUILD_EXAMPLES OFF CACHE BOOL "Set to ON to build examples")
#set(CPPTERMINAL_ENABLE_INSTALL OFF CACHE BOOL "Set to ON to enable install")
set(CPPTERMINAL_ENABLE_TESTING OFF CACHE BOOL "Set to ON to enable testing")
set(CPPTERMINAL_ENABLE_DOCS OFF CACHE BOOL "Set to ON to generate documentation")

FetchContent_MakeAvailable(cpp-terminal)
set(TERMINALLIB cpp-terminal)

vidre_deps_set(cpp-terminal ON) # always fetched