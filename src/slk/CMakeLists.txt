add_library(slk
    color.cpp
    input/input.cpp
    math/curves.cpp
    math/math_constants.cpp)

target_compile_options(slk PRIVATE
    -Werror
    -Wall 
    -Wextra

    -Wno-gnu-anonymous-struct
    -Wno-nested-anon-types
    -Wno-unknown-pragmas)

# specify extern libs as System includes to avoid/silent compilation warnings
target_include_directories(slk SYSTEM PRIVATE
    ${SLK_EXTERN_INCLUDE_DIR})

target_include_directories(slk PUBLIC
    ${SLK_SRC_DIR})

set_target_properties(slk PROPERTIES
    CXX_STANDARD 26
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF)

