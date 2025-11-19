# test if a string is in a list
# slk_is_in_list("banana" FOUND "apple" "banana" "orange")
#   if(FOUND)
#      ...
#   endif()
function(slk_is_in_list ITEM RESULT)
    set(MY_LIST ${ARGN})  # Remaining args become the list
    if(ITEM IN_LIST MY_LIST)
        set(${RESULT} TRUE PARENT_SCOPE)
    else()
        set(${RESULT} FALSE PARENT_SCOPE)
    endif()
endfunction()

# cmake add_executable wrapper adding extra target dependencies and post-build commands required by certain platforms e.g. PS3
function(slk_add_executable target_name)
    add_executable(${target_name} ${ARGN})

    if(${SLK_TARGET_PLATFORM_ID} STREQUAL "ps3")
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
    endif()

endfunction()

