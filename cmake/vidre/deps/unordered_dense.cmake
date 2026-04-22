# Package: unordered_dense
# Version: 4.8.1
#

# if(VIDRE_DEPS_CHECK_ONLY)
#     find_package(unordered_dense CONFIG)
# 	vidre_deps_set(unordered_dense ${unordered_dense_FOUND} REQUIRED)
# elseif(NOT ${unordered_dense_FOUND})
    FetchContent_Declare(unordered_dense
        GIT_REPOSITORY https://github.com/martinus/unordered_dense.git
		GIT_TAG        v4.8.1
        GIT_SHALLOW    TRUE
        GIT_PROGRESS   ON
        FIND_PACKAGE_ARGS
            CONFIG
    )
    
    FetchContent_MakeAvailable(unordered_dense)
	vidre_deps_set(unordered_dense ON)

    install(
        TARGETS unordered_dense
        EXPORT VidreTargets-${Vidre_RELEASE}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

    install(
        DIRECTORY ${unordered_dense_SOURCE_DIR}/include/ankerl
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

# endif()
