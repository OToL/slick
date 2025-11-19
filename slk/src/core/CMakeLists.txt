add_library(core
    color.cpp
    math/math_constants.cpp)

add_library(slk::core ALIAS core)

target_compile_options(core PRIVATE
    -Werror
    -Wall 
    -Wextra

    -Wno-gnu-anonymous-struct
    -Wno-nested-anon-types
    -Wno-unknown-pragmas)

# specify extern libs as System includes to avoid/silent compilation warnings
target_include_directories(core SYSTEM PRIVATE
    ${SLK_EXTERN_INCLUDE_DIR})

target_include_directories(core PUBLIC
    ${SLK_SRC_DIR})

set_target_properties(core PROPERTIES
    CXX_STANDARD 26
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF)

