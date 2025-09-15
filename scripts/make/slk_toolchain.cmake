#[[ 
if(((${CMAKE_SYSTEM_PROCESSOR} STREQUAL "i386") AND (CMAKE_SIZEOF_VOID_P EQUAL 8)) OR
    (${CMAKE_SYSTEM_PROCESSOR} STREQUAL "x86_64") OR 
    (${CMAKE_SYSTEM_PROCESSOR} STREQUAL "AMD64"))
    set(TARGET_CPU_ARCH_ID "64")
    set(TARGET_CPU_FAMILY_NAME "x86")
    set(TARGET_CPU_ID "x86_64")
    set(TARGET_CPU_ARCH_64B TRUE)
else()
    message(FATAL_ERROR "Unsupported CPU: ${CMAKE_SYSTEM_PROCESSOR}-${CMAKE_SIZEOF_VOID_P}")
endif()

if(${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
    set(TARGET_PLATFORM_ID "macos")
    set(TARGET_PLATFORM_FAMILY_ID "unix")
    set(TARGET_PLATFORM_FILE_SUFFIX "macos") 
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    set(TARGET_PLATFORM_ID "linux")	
    set(TARGET_PLATFORM_FAMILY_ID "unix")
    set(TARGET_PLATFORM_FILE_SUFFIX "linux") 
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    set(TARGET_PLATFORM_ID "windows")	
    set(TARGET_PLATFORM_FAMILY_ID "windows")
    set(TARGET_PLATFORM_FILE_SUFFIX "win") 
else()
    message(FATAL_ERROR "Unknown target: ${CMAKE_SYSTEM_NAME}")
endif()

set(TARGET_PLATFORM_FULL_ID ${TARGET_PLATFORM_ID}_${TARGET_CPU_ID}) 
]]

if(${SLK_TARGET_PLATFORM_ID} STREQUAL "ps3")
    include(scripts/make/slk_toolchain_ps3_gcc.cmake)
else()

endif()
