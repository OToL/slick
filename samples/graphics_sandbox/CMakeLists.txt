add_executable(graphics_sandbox
    main.cpp)

target_link_libraries(graphics_sandbox 
    slk::ext-raylib)

target_include_directories(graphics_sandbox PRIVATE
    ${SLK_EXTERN_INCLUDE_DIR})

set_target_properties(graphics_sandbox PROPERTIES
    CXX_STANDARD 26
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF)

