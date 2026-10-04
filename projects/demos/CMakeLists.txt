include(${SLICK_EXTERN_EXTRA_DIR}/bgfx/scripts/bgfx_tool_utils.cmake)

bgfx_compile_shaders(
	TYPE VERTEX
    VARYING_DEF ${CMAKE_CURRENT_LIST_DIR}/data/shaders/varying.def.sc
    OUTPUT_DIR ${CMAKE_CURRENT_LIST_DIR}/_build/shaders
    INCLUDE_DIRS ${SLICK_EXTERN_EXTRA_DIR}/bgfx/shaders
	SHADERS data/shaders/grid_vs.sc
    DEPENDS 
        ${SLICK_EXTERN_EXTRA_DIR}/bgfx/shaders/bgfx.sh 
        ${SLICK_EXTERN_EXTRA_DIR}/bgfx/shaders/bgfx_shader_lib.sh 
        ${SLICK_EXTERN_EXTRA_DIR}/bgfx/shaders/bgfx_shader.sh)

bgfx_compile_shaders(
    TYPE FRAGMENT
    VARYING_DEF ${CMAKE_CURRENT_LIST_DIR}/data/shaders/varying.def.sc
    OUTPUT_DIR ${CMAKE_CURRENT_LIST_DIR}/_build/shaders
    INCLUDE_DIRS ${SLICK_EXTERN_EXTRA_DIR}/bgfx/shaders
	SHADERS data/shaders/grid_fs.sc
    DEPENDS
        ${SLICK_EXTERN_EXTRA_DIR}/bgfx/shaders/bgfx.sh ${SLICK_EXTERN_EXTRA_DIR}/bgfx/shaders/bgfx_shader_lib.sh 
        ${SLICK_EXTERN_EXTRA_DIR}/bgfx/shaders/bgfx_shader.sh)

add_executable(demos
    src/graphics.cpp
    src/demos.cpp
    src/demo_empty.cpp
    src/demo_curves.cpp
    src/demo_picking.cpp
    src/app.cpp
    src/main_${SLICK_TARGET_PLATFORM_ID}.cpp
    data/shaders/grid_fs.sc
    data/shaders/grid_vs.sc
)

if(${SLICK_TARGET_PLATFORM_ID} STREQUAL "darwin")
    set_source_files_properties(src/main_${SLICK_TARGET_PLATFORM_ID}.cpp PROPERTIES
          COMPILE_FLAGS "-x objective-c++")
endif()

target_link_libraries(demos 
    slk
    bgfx-utils
    ext-sokol 
    ext-bx 
    ext-bgfx 
    ext-bimg)

target_compile_options(demos PRIVATE
    -Werror
    -Wall 
    -Wextra

    -Wno-unused-function
    -Wno-gnu-anonymous-struct
    -Wno-nested-anon-types
    -Wno-unknown-pragmas)

# specify extern libs as System includes to avoid/silent compilation warnings
target_include_directories(demos SYSTEM PRIVATE
    ${SLICK_EXTERN_INCLUDE_DIR}
    "${SLICK_EXTERN_INCLUDE_DIR}/bgfx/3rdparty")

set_target_properties(demos PROPERTIES
    CXX_STANDARD 26
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF)

