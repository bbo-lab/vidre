
# Dynamic loader sources
set(DYNAMIC_LOADER_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/dynamic_library_loader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/dynamic_library_loader.hpp
)

# Add to vidre target if defined
if(TARGET vidre)
    target_sources(vidre PRIVATE ${DYNAMIC_LOADER_SRC})
endif()
