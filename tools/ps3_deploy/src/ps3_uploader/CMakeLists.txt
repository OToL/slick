add_executable(ps3_uploader
    main.cpp)

set_target_properties(ps3_uploader PROPERTIES
    CXX_STANDARD 23
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF)

target_include_directories(ps3_uploader PRIVATE 
    ${SLK_SRC_DIR}
    ${SLK_EXTERN_INC_DIR})

target_link_libraries(ps3_uploader PRIVATE 
    slk::ext-zlib)
