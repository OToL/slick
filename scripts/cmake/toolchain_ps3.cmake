set(SLK_TARGET_PLATFORM_ID ps3)

# Locate PS3 SDK
set(SLK_PS3_SDK_ROOT_DIR_PATH $ENV{PSL1GHT})
if(NOT SLK_PS3_SDK_ROOT_DIR_PATH)
    message(FATAL_ERROR "Cannot find 'PSL1GHT' environment variable")
endif()

# PS3 tools
set (SLK_PS3_TOOL_C_COMPILER    ${SLK_PS3_SDK_ROOT_DIR_PATH}/ppu/bin/ppu-gcc)
set (SLK_PS3_TOOL_CXX_COMPILER  ${SLK_PS3_SDK_ROOT_DIR_PATH}/ppu/bin/ppu-c++)
set (SLK_PS3_TOOL_AR            ${SLK_PS3_SDK_ROOT_DIR_PATH}/ppu/bin/ppu-ar)
set (SLK_PS3_TOOL_LINKER        ${SLK_PS3_SDK_ROOT_DIR_PATH}/ppu/bin/ppu-ld)
set (SLK_PS3_TOOL_STRIP         ${SLK_PS3_SDK_ROOT_DIR_PATH}/ppu/bin/ppu-strip)
set (SLK_PS3_TOOL_OBJCOPY       ${SLK_PS3_SDK_ROOT_DIR_PATH}/ppu/bin/ppu-objcopy)
set (SLK_PS3_TOOL_SPRXLINKER    ${SLK_PS3_SDK_ROOT_DIR_PATH}/bin/sprxlinker)
set (SLK_PS3_TOOL_MAKESELF      ${SLK_PS3_SDK_ROOT_DIR_PATH}/bin/make_self)

# Common library & include PS3 SDK directories
set(SLK_PS3_SDK_LIB_DIRS_PATH 
    ${SLK_PS3_SDK_ROOT_DIR_PATH}/ppu/lib 
    ${SLK_PS3_SDK_ROOT_DIR_PATH}/portlibs/ppu/lib)
set(SLK_PS3_SDK_INCLUDE_DIRS_PATH 
    ${SLK_PS3_SDK_ROOT_DIR_PATH}/ppu/include 
    ${SLK_PS3_SDK_ROOT_DIR_PATH}/portlibs/ppu/include)

# CMake system information
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR ppu) 

# Set CMake tools from PS3 SDK
set(CMAKE_AR ${SLK_PS3_TOOL_AR})
set(CMAKE_ASM_COMPILER ${SLK_PS3_TOOL_AR})
set(CMAKE_C_COMPILER ${SLK_PS3_TOOL_C_COMPILER})
set(CMAKE_CXX_COMPILER ${SLK_PS3_TOOL_CXX_COMPILER})
set(CMAKE_LINKER ${SLK_PS3_TOOL_LINKER})

# Instruct CMake to skip compiler tests
set(CMAKE_C_COMPILER_WORKS TRUE CACHE INTERNAL "C compiler works")
set(CMAKE_CXX_COMPILER_WORKS TRUE CACHE INTERNAL "CXX compiler works")
set(CMAKE_CROSSCOMPILING TRUE)

# Override executables binary extension
set(CMAKE_EXECUTABLE_SUFFIX_ASM .elf)
set(CMAKE_EXECUTABLE_SUFFIX_C   .elf)
set(CMAKE_EXECUTABLE_SUFFIX_CXX .elf)

# Common compile flags for Homebrew LLVM on macOS
#   -mcpu=cell: Target the Cell processor (used in PS3, based on PowerPC). Enables specific tuning and instructions.
#   -mhard-float: Use hardware floating-point instructions (instead of software emulation). Must match target ABI. 
#   -fmodulo-sched: Enable modulo scheduling: advanced loop optimization technique, useful for VLIW and DSP-like CPUs.
#   -ffunction-sections: Put each function in its own section (e.g., .text.funcname)—enables linker garbage collection.
#   -fdata-sections: Put each data variable in its own section — same idea as ffunction-sections, for data like globals.
#   -O2: Attempts to reduce both the size of the output binary code and the execution speed, but it does not perform optimizations 
#        that might substantially increase the compilation time.
set(COMMON_COMPILATION_FLAGS "-O2 -mcpu=cell -mhard-float -fmodulo-sched -ffunction-sections -fdata-sections ")
set(CMAKE_EXE_LINKER_FLAGS "-mhard-float -fmodulo-sched -ffunction-sections -fdata-sections")
set(CMAKE_C_FLAGS "${COMMON_COMPILATION_FLAGS}")
set(CMAKE_CXX_FLAGS "${COMMON_COMPILATION_FLAGS}")

