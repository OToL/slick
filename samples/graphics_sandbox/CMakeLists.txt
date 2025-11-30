add_executable(graphics_sandbox
    src/graphics.cpp
    src/demos.cpp
    src/demo_empty.cpp
    src/demo_curves.cpp
    src/main.cpp)

target_link_libraries(graphics_sandbox 
    slk::core
    slk::utl-raylib
    slk::ext-raylib
    slk::ext-imgui
    slk::ext-rlimgui)

target_compile_options(graphics_sandbox PRIVATE
    -Werror
    -Wall 
    -Wextra

    -Wno-unused-function
    -Wno-gnu-anonymous-struct
    -Wno-nested-anon-types
    -Wno-unknown-pragmas)

# specify extern libs as System includes to avoid/silent compilation warnings
target_include_directories(graphics_sandbox SYSTEM PRIVATE
    ${SLK_EXTERN_INCLUDE_DIR})

set_target_properties(graphics_sandbox PROPERTIES
    CXX_STANDARD 26
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF)

