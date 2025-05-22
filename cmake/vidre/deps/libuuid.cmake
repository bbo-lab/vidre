# UUID
check_include_file_cxx("uuid/uuid.h" Vidre_HAVE_LIBUUID)
if(Vidre_HAVE_LIBUUID)
    set(UUIDLIB uuid)
else()
    message(FATAL "Could not find <uuid/uuid.h>")
endif()

