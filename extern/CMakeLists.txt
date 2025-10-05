########################################################################################################################
#                                            External Dependencies                                                     #
########################################################################################################################

# NOTE: public includes are always taken from redist: extern\redist\include\<dep name>.
#       this means they have to be updated whenever associated sources are modified (extern\src\<dep name>).

# We don't care about cmake errors/warnings from external dependencies
cmake_policy(PUSH)
set(CMAKE_SUPPRESS_DEVELOPER_WARNINGS ON)

# ZLIB
set(ZLIB_BUILD_SHARED OFF CACHE BOOL "" FORCE)
set(ZLIB_BUILD_TESTING OFF CACHE BOOL "" FORCE)

if(SLK_ENABLE_BUILD_EXTERN)
    add_subdirectory(src/zlib)
    add_library(slk::ext-zlib ALIAS zlibstatic)
else()
    add_library(slk_extern_redist_zlib STATIC IMPORTED)
    set_target_properties(slk_extern_redist_zlib PROPERTIES
        IMPORTED_LOCATION_DEBUG "${SLK_EXTERN_LIB_DIR}//macos/libz_debug.a")
    add_library(slk_extern_zlib INTERFACE)
    add_library(slk::ext-zlib ALIAS slk_extern_zlib)
    target_link_libraries(slk_extern_zlib
        INTERFACE
            slk_extern_redist_zlib)
endif()

# RayLib
if(${SLK_TARGET_PLATFORM_ID} STREQUAL "darwin")
    if(SLK_ENABLE_BUILD_EXTERN)
        add_subdirectory(src/raylib)
        add_library(slk::ext-raylib ALIAS raylib)
    else()
        add_library(slk_extern_redist_raylib STATIC IMPORTED)
        set_target_properties(slk_extern_redist_raylib PROPERTIES
            IMPORTED_LOCATION_DEBUG "${SLK_EXTERN_LIB_DIR}//macos/libraylib_debug.a")
        if(${SLK_TARGET_PLATFORM_ID} STREQUAL "darwin")
            target_link_libraries(slk_extern_redist_raylib INTERFACE
                  "-framework Cocoa"
                  "-framework OpenGL"
                  "-framework IOKit"
                  "-framework CoreVideo")
        endif()
        add_library(slk_extern_raylib INTERFACE)
        add_library(slk::ext-raylib ALIAS slk_extern_raylib)
        target_link_libraries(slk_extern_raylib
            INTERFACE
                slk_extern_redist_raylib)
    endif()
endif()

cmake_policy(POP)
