add_executable(asteroids
    cannon.cpp
    graphics.cpp
    app.cpp
    main_${SLICK_TARGET_PLATFORM_ID}.cpp)

if(${SLICK_TARGET_PLATFORM_ID} STREQUAL "darwin")
    set_source_files_properties(main_${SLICK_TARGET_PLATFORM_ID}.cpp PROPERTIES
          COMPILE_FLAGS "-x objective-c++")
endif()

target_compile_options(asteroids PRIVATE
    -Werror
    -Wall
    -Wextra

    -Wno-gnu-anonymous-struct
    -Wno-nested-anon-types
    -Wno-unknown-pragmas)

target_link_libraries(asteroids
    slk
    bgfx-utils
    ext-sokol
    ext-bx
    ext-bgfx
    ext-bimg)

# specify extern libs as System includes to avoid/silent compilation warnings
target_include_directories(asteroids SYSTEM PRIVATE
    ${SLICK_EXTERN_INCLUDE_DIR})

target_include_directories(asteroids PRIVATE
    ${SLICK_SRC_DIR})

set_target_properties(asteroids PROPERTIES
    CXX_STANDARD 26
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF)
