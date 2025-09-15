########################################################################################################################
#                                            External Dependencies                                                     #
########################################################################################################################

# NOTE: public includes are always taken from redist: extern\redist\include\<dep name>.
#       this means they have to be updated whenever associated sources are modified (extern\src\<dep name>).

# ZLIB
set(ZLIB_BUILD_SHARED OFF CACHE BOOL "" FORCE)
set(ZLIB_BUILD_TESTING OFF CACHE BOOL "" FORCE)

if(SLK_ENABLE_BUILD_EXTERN)
    add_subdirectory(src/zlib)
    add_library(slk::ext-zlib ALIAS zlibstatic)
else()
    add_library(slk_extern_redist_zlib STATIC IMPORTED)
    set_target_properties(slk_extern_redist_zlib PROPERTIES
        IMPORTED_LOCATION_DEBUG "${SLK_EXTERN_LIB_DIR}//macos/libz_debug.a"
    )
    add_library(slk_extern_zlib INTERFACE)
    add_library(slk::ext-zlib ALIAS slk_extern_zlib)
    target_link_libraries(slk_extern_zlib
        INTERFACE
            slk_extern_redist_zlib)
endif()
