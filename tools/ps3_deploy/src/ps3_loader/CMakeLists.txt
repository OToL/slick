slk_add_executable_ps3(ps3_loader     
    main.c
    pad.c
    rsxutil.c)

target_include_directories(ps3_loader PRIVATE 
    ${SLK_PS3_SDK_INCLUDE_DIRS_PATH}
    ${SLK_3RD_INC_DIR})

target_link_directories(ps3_loader PUBLIC 
    ${SLK_PS3_SDK_LIB_DIRS_PATH})

target_link_libraries(ps3_loader PRIVATE 
    zip z net rsx gcm_sys io sysmodule sysutil rt lv2 m
    slk::3rd-zlib)
