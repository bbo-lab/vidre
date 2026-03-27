find_package(Doxygen)

if(Doxygen_FOUND)

    FetchContent_Populate(doxygen_awesome_css QUIET
        GIT_REPOSITORY https://github.com/jothepro/doxygen-awesome-css
        GIT_TAG        v2.3.3
        GIT_PROGRESS   ON
        SOURCE_DIR "${CMAKE_CURRENT_BINARY_DIR}/doxygen-awesome-css"
    )
    # FIXME disable patch/configure/etc. steps

    set(doxyfile_in ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in)
    set(doxyfile ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

    configure_file(${doxyfile_in} ${doxyfile} @ONLY)

    add_custom_target(docs
        COMMAND ${DOXYGEN_EXECUTABLE} ${doxyfile}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating API documentation with Doxygen"
        DEPENDS ${doxyfile_in}
        VERBATIM)

    install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/html 
        COMPONENT "documentation" # FIXME sub-category html?
        DESTINATION ${CMAKE_INSTALL_DOCDIR}/html)

endif()