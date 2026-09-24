# Package: ftxui
# Version: 7.0.3
#

if(VIDRE_DEPS_CHECK_ONLY)
	find_package(ftxui CONFIG QUIET)
	vidre_deps_set(ftxui ${ftxui_FOUND} REQUIRED)
elseif(NOT DEFINED FTXUI)
    message(STATUS "Fetching ArthurSonzogni/FTXUI...")

    FetchContent_Declare(ftxui
        GIT_REPOSITORY https://github.com/ArthurSonzogni/FTXUI.git
		GIT_TAG        v7.0.3
        GIT_SHALLOW    TRUE
        GIT_PROGRESS   ON
        FIND_PACKAGE_ARGS
    )

    set(FTXUI_ENABLE_INSTALL OFF)

    FetchContent_MakeAvailable(ftxui)

	set(FTXUI ftxui::ftxui)
    # FIXME install(TARGETS ftxui::ftxui DESTINATION lib)
	vidre_deps_set(ftxui ON)
endif()
