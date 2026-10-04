########################################################################################################################
#                                            External Dependencies                                                     #
########################################################################################################################

# NOTE: public includes are always taken from redist: extern\redist\include\<dep name>.
#       this means they have to be updated whenever associated sources are modified (extern\src\<dep name>).

slk_is_in_list(${SLICK_TARGET_PLATFORM_ID} ENABLE_EXTERN_ZLIB     "darwin" "ps3")
slk_is_in_list(${SLICK_TARGET_PLATFORM_ID} ENABLE_EXTERN_BGFX     "darwin")
slk_is_in_list(${SLICK_TARGET_PLATFORM_ID} ENABLE_EXTERN_SOKOL    "darwin")

# ZLIB
set(ZLIB_BUILD_SHARED OFF CACHE BOOL "" FORCE)
set(ZLIB_BUILD_TESTING OFF CACHE BOOL "" FORCE)

if(ENABLE_EXTERN_ZLIB)
    if(SLICK_ENABLE_BUILD_EXTERN)
        add_subdirectory(src/zlib)
        add_library(ext-zlib ALIAS zlibstatic)
    else()
        add_library(slk_extern_redist_zlib STATIC IMPORTED)
        set_target_properties(slk_extern_redist_zlib PROPERTIES
            IMPORTED_LOCATION_DEBUG "${SLICK_EXTERN_LIB_DIR}/darwin/libz_debug.a")
        add_library(slk_extern_zlib INTERFACE)
        add_library(ext-zlib ALIAS slk_extern_zlib)
        target_link_libraries(slk_extern_zlib
            INTERFACE
                slk_extern_redist_zlib)
    endif()
endif()

if(ENABLE_EXTERN_SOKOL)
    add_library(sokol INTERFACE)
    add_library(ext-sokol ALIAS sokol)
    # Add any defines needed
    target_compile_definitions(sokol INTERFACE
      SOKOL_METAL
      SOKOL_NO_ENTRY
    )
    # Platform-specific linking
    if(${SLICK_TARGET_PLATFORM_ID} STREQUAL "darwin")
        target_link_libraries(sokol INTERFACE
          "-framework Cocoa"
          "-framework QuartzCore"
          "-framework Metal"
          "-framework MetalKit")
    endif()
endif()

if(ENABLE_EXTERN_BGFX)
    if(SLICK_ENABLE_BUILD_EXTERN)
        add_subdirectory(src/bgfx.cmake)
        add_library(ext-bx ALIAS bx)
        add_library(ext-bgfx ALIAS bgfx)
        add_library(ext-bimg ALIAS bimg)
    else()
        add_library(slk_extern_redist_bx STATIC IMPORTED)
        set_target_properties(slk_extern_redist_bx PROPERTIES
            IMPORTED_LOCATION_DEBUG "${SLICK_EXTERN_LIB_DIR}/darwin/libbx_debug.a")
        add_library(slk_extern_bx INTERFACE)
        add_library(ext-bx ALIAS slk_extern_bx)
        target_compile_definitions(slk_extern_bx INTERFACE
             $<$<CONFIG:Debug>:BX_CONFIG_DEBUG=1>
             $<$<CONFIG:RelWithDebInfo>:BX_CONFIG_DEBUG=1>
             $<$<CONFIG:Release>:BX_CONFIG_DEBUG=0>
          )
        target_link_libraries(slk_extern_bx
            INTERFACE
                slk_extern_redist_bx) 
        target_include_directories(slk_extern_bx
            INTERFACE
                "${SLICK_EXTERN_INCLUDE_DIR}/bx/include")

        add_library(slk_extern_redist_bgfx STATIC IMPORTED)
        set_target_properties(slk_extern_redist_bgfx PROPERTIES
            IMPORTED_LOCATION_DEBUG "${SLICK_EXTERN_LIB_DIR}/darwin/libbgfx_debug.a")
        add_library(slk_extern_bgfx INTERFACE)
        add_library(ext-bgfx ALIAS slk_extern_bgfx)
        target_link_libraries(slk_extern_bgfx
            INTERFACE
                slk_extern_redist_bgfx) 
        target_include_directories(slk_extern_bgfx
            INTERFACE
                "${SLICK_EXTERN_INCLUDE_DIR}/bgfx/include")
        if(${SLICK_TARGET_PLATFORM_ID} STREQUAL "darwin")
            target_link_libraries(slk_extern_redist_bgfx INTERFACE
                  "-framework Cocoa"
                  "-framework OpenGL"
                  "-framework IOKit"
                  "-framework CoreVideo")
        endif()

        add_library(slk_extern_redist_bimg STATIC IMPORTED)
        set_target_properties(slk_extern_redist_bimg PROPERTIES
            IMPORTED_LOCATION_DEBUG "${SLICK_EXTERN_LIB_DIR}/darwin/libbimg_debug.a")
        add_library(slk_extern_bimg INTERFACE)
        add_library(ext-bimg ALIAS slk_extern_bimg)
        target_link_libraries(slk_extern_bimg
            INTERFACE
                slk_extern_redist_bimg) 
        target_include_directories(slk_extern_bimg
            INTERFACE
                "${SLICK_EXTERN_INCLUDE_DIR}/bimg/include")
    endif()
endif()

