add_library(mymodule MODULE mymodule.cc)
set_target_properties(mymodule PROPERTIES PREFIX "")
target_link_libraries(mymodule Vidre::Base)
