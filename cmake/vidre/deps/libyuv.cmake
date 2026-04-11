# Package: libyuv
# Version: 0.0
#
# Ubuntu:
# - libyuv-dev

if(VIDRE_DEPS_CHECK_ONLY)
	find_package(libyuv CONFIG QUIET)

	if(TARGET libyuv::libyuv)
		set(YUVLIB libyuv::libyuv)
	elseif(TARGET yuv)
		set(YUVLIB yuv)
	else()
		if(PkgConfig_FOUND)
			pkg_check_modules(LIBYUV QUIET IMPORTED_TARGET libyuv)
		endif()

		if(TARGET PkgConfig::LIBYUV)
			set(YUVLIB PkgConfig::LIBYUV)
		else()
			# Last-resort discovery for systems without CMake or pkg-config metadata.
			find_path(LIBYUV_INCLUDE_DIR
				NAMES libyuv.h
				PATH_SUFFIXES libyuv
			)

			find_library(LIBYUV_LIBRARY
				NAMES yuv libyuv
			)

			if(LIBYUV_INCLUDE_DIR AND LIBYUV_LIBRARY)
				if(NOT TARGET libyuv::libyuv)
					add_library(libyuv::libyuv UNKNOWN IMPORTED)
					set_target_properties(libyuv::libyuv PROPERTIES
						IMPORTED_LOCATION "${LIBYUV_LIBRARY}"
						INTERFACE_INCLUDE_DIRECTORIES "${LIBYUV_INCLUDE_DIR}"
					)
				endif()
				set(YUVLIB libyuv::libyuv)
			endif()
		endif()
	endif()

	if(DEFINED YUVLIB)
		set(libyuv_FOUND ON)
	endif()
	vidre_deps_set(libyuv ${libyuv_FOUND} REQUIRED)
elseif(NOT DEFINED YUVLIB)
    FetchContent_Declare(libyuv
        GIT_REPOSITORY https://chromium.googlesource.com/libyuv/libyuv/
        GIT_SHALLOW    TRUE
        GIT_PROGRESS   ON
        FIND_PACKAGE_ARGS
    )
    FetchContent_MakeAvailable(libyuv)
	set(YUVLIB libyuv::libyuv)
endif()
