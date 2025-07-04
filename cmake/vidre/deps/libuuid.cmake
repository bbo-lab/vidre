# UUID
check_include_file_cxx("uuid/uuid.h" VIDRE_HAVE_LIBUUID)
if(VIDRE_HAVE_LIBUUID)
    set(UUIDLIB uuid)
endif()

