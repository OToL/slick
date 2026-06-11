set(bgfx_internal_files 
    "${SLICK_EXTERN_INCLUDE_DIR}/bgfx/3rdparty/dear-imgui/imgui.cpp"
    "${SLICK_EXTERN_INCLUDE_DIR}/bgfx/3rdparty/dear-imgui/imgui_demo.cpp"
    "${SLICK_EXTERN_INCLUDE_DIR}/bgfx/3rdparty/dear-imgui/imgui_draw.cpp"
    "${SLICK_EXTERN_INCLUDE_DIR}/bgfx/3rdparty/dear-imgui/imgui_tables.cpp"
    "${SLICK_EXTERN_INCLUDE_DIR}/bgfx/3rdparty/dear-imgui/imgui_widgets.cpp")

add_library(bgfx-utils
    imgui/imgui.cpp
    ${bgfx_internal_files})

target_compile_options(bgfx-utils PRIVATE
    -Werror
    -Wall 
    -Wextra

    -Wno-gnu-anonymous-struct
    -Wno-nested-anon-types
    -Wno-unknown-pragmas)

# disable compilation warnings for external files
set_source_files_properties(${bgfx_internal_files} PROPERTIES
      COMPILE_FLAGS "-w")

# specify extern libs as System includes to avoid/silent compilation warnings
target_include_directories(bgfx-utils SYSTEM 
    PUBLIC
        ${SLICK_EXTERN_INCLUDE_DIR}
    PRIVATE
        "${SLICK_EXTERN_INCLUDE_DIR}/bgfx/3rdparty")

target_link_libraries(bgfx-utils PUBLIC
    ext-bgfx
    ext-bimg
    ext-bx)

set_target_properties(bgfx-utils PROPERTIES
    CXX_STANDARD 26
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF)

