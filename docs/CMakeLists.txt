find_package(Doxygen 1.18)

if(Doxygen_FOUND)

    FetchContent_Declare(doxygen-awesome-css QUIET
        GIT_REPOSITORY https://github.com/jothepro/doxygen-awesome-css
        GIT_TAG        v2.5.0
        GIT_PROGRESS   ON
        GIT_SHALLOW    ON
        SOURCE_DIR "${CMAKE_CURRENT_BINARY_DIR}/doxygen-awesome-css"
    )
    FetchContent_MakeAvailable(doxygen-awesome-css)
    FetchContent_GetProperties(doxygen-awesome-css SOURCE_DIR AWESOME_CSS_DIR)

    set(doxyfile_in ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in)
    set(doxyfile ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

    configure_file(${doxyfile_in} ${doxyfile} @ONLY)

    add_custom_target(docs
        COMMAND ${DOXYGEN_EXECUTABLE} ${doxyfile}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating API documentation with Doxygen"
        DEPENDS ${doxyfile_in}
        VERBATIM)

    set_target_properties(docs PROPERTIES
        ADDITIONAL_CLEAN_FILES ${CMAKE_CURRENT_BINARY_DIR}/html
    )

    install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/html 
        COMPONENT "documentation" # FIXME sub-category html?
        DESTINATION ${CMAKE_INSTALL_DOCDIR}/html)

    if (NOT "$ENV{BBOLAB_GITHUB_IO}" STREQUAL "")
        add_custom_target(publish-prepare
            WORKING_DIRECTORY $ENV{BBOLAB_GITHUB_IO}
            COMMAND git checkout .
            COMMAND git pull
            VERBATIM
        )

        add_custom_target(publish 
            COMMAND ${CMAKE_COMMAND} -E echo "Removing existing documentation at $ENV{BBOLAB_GITHUB_IO}/vidre..."
            COMMAND ${CMAKE_COMMAND} -E remove_directory $ENV{BBOLAB_GITHUB_IO}/vidre
            COMMAND ${CMAKE_COMMAND} -E echo "Copying new documentation..."
            COMMAND ${CMAKE_COMMAND} -E copy_directory
                ${CMAKE_CURRENT_BINARY_DIR}/html
                $ENV{BBOLAB_GITHUB_IO}/vidre
            COMMAND ${CMAKE_COMMAND} -E echo "Documentation copied successfully!"
            DEPENDS 
                ${CMAKE_CURRENT_BINARY_DIR}/html/index.html
                publish-prepare
            COMMENT "Deploying documentation to $ENV{BBOLAB_GITHUB_IO}/vidre"
            VERBATIM
        )

        add_custom_command(TARGET publish POST_BUILD
            WORKING_DIRECTORY $ENV{BBOLAB_GITHUB_IO}
            COMMAND git add vidre
            COMMAND git commit -m "update vidre ${Vidre_VERSION_MAJOR}.${Vidre_VERSION_MINOR} documentation"
            COMMAND git push
            COMMAND ${CMAKE_COMMAND} -E echo "Documentation published successfully!"
            COMMENT "Published documentation to $ENV{BBOLAB_GITHUB_IO}"
            VERBATIM
        )
    endif()
endif()
