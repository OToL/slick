add_executable(asteroids
    cannon.cpp
    graphics.cpp
    rlib_utils.cpp
    main.cpp)

target_compile_options(asteroids PRIVATE
    -Werror
    -Wall 
    -Wextra

    -Wno-gnu-anonymous-struct
    -Wno-nested-anon-types
    -Wno-unknown-pragmas)

target_link_libraries(asteroids 
    slk::core
    slk::ext-raylib)

# specify extern libs as System includes to avoid/silent compilation warnings
target_include_directories(asteroids SYSTEM PRIVATE
    ${SLK_EXTERN_INCLUDE_DIR})

target_include_directories(asteroids PRIVATE
    ${SLK_SRC_DIR})

set_target_properties(asteroids PROPERTIES
    CXX_STANDARD 26
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF)

