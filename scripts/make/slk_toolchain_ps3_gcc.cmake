function(slk_add_executable_ps3 target_name)
    add_executable(${target_name} ${ARGN})

   target_link_options(${target_name} PUBLIC 
       -Wl,-Map,$<TARGET_FILE:${target_name}>.map)

    add_custom_command(
        TARGET ${target_name}
        POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E echo "Executing postbuild steps ..."
            COMMAND ${CMAKE_COMMAND} -E env ${SLK_PS3_TOOL_STRIP} $<TARGET_FILE:${target_name}> -o $<TARGET_FILE:${target_name}>.stripped
            COMMAND ${CMAKE_COMMAND} -E env ${SLK_PS3_TOOL_SPRXLINKER} $<TARGET_FILE:${target_name}>.stripped
            COMMAND ${CMAKE_COMMAND} -E env ${SLK_PS3_TOOL_MAKESELF} $<TARGET_FILE:${target_name}>.stripped $<TARGET_FILE_DIR:${target_name}>/${target_name}.self
    )

endfunction()

