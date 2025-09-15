########################################################################################################################
#                                            External Dependencies                                                     #
########################################################################################################################

# ZLIB
set(ZLIB_BUILD_SHARED OFF CACHE BOOL "" FORCE)
set(ZLIB_BUILD_TESTING OFF CACHE BOOL "" FORCE)

if(SLK_ENABLE_BUILD_EXTERN)
    add_subdirectory(src/zlib)
    add_library(slk::ext-zlib ALIAS zlibstatic)
else()
    add_library(slk_ext_redis_zlib STATIC IMPORTED)
    set_target_properties(slk_ext_redis_zlib PROPERTIES
        IMPORTED_LOCATION_DEBUG "/Users/emmanuelbeau/Documents/dev/slick/extern/redis/lib/macos/libz_debug.a"
        # IMPORTED_LOCATION_RELEASE "redis/lib/macos/libz_release.a"
    )
    add_library(slk_ext_zlib INTERFACE)
    add_library(slk::ext-zlib ALIAS slk_ext_zlib)
    target_link_libraries(slk_ext_zlib
        INTERFACE
            slk_ext_redis_zlib)
endif()
