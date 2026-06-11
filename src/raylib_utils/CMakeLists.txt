add_library(raylib-utils
    conversions.cpp)

target_compile_options(raylib-utils PRIVATE
    -Werror
    -Wall 
    -Wextra

    -Wno-gnu-anonymous-struct
    -Wno-nested-anon-types
    -Wno-unknown-pragmas)

# specify extern libs as System includes to avoid/silent compilation warnings
target_include_directories(raylib-utils SYSTEM PUBLIC
    ${SLICK_EXTERN_INCLUDE_DIR})

target_include_directories(raylib-utils PUBLIC
    ${SLICK_UTILS_DIR})

target_link_libraries(raylib-utils PUBLIC
    ext-raylib
    slk)

set_target_properties(raylib-utils PROPERTIES
    CXX_STANDARD 26
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF)

