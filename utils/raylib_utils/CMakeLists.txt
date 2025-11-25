add_library(raylib_utils
    conversion.cpp)
add_library(slk::utils-raylib ALIAS raylib_utils)

target_compile_options(raylib_utils PRIVATE
    -Werror
    -Wall 
    -Wextra

    -Wno-gnu-anonymous-struct
    -Wno-nested-anon-types
    -Wno-unknown-pragmas)

# specify extern libs as System includes to avoid/silent compilation warnings
target_include_directories(raylib_utils SYSTEM PUBLIC
    ${SLK_EXTERN_INCLUDE_DIR})

target_include_directories(raylib_utils PUBLIC
    ${SLK_UTILS_DIR})

target_link_libraries(raylib_utils PUBLIC
    slk::ext-raylib
    slk::core)

set_target_properties(raylib_utils PROPERTIES
    CXX_STANDARD 26
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF)

