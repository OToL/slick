set(SLK_TARGET_PLATFORM_ID darwin)

# When setting this variable to gcc, remove -nostdinc++ from .clangd
set(SLK_TOOLCHAIN_NAME "clang" CACHE STRING "")

# Locate the macOS SDK path dynamically
execute_process(
  COMMAND xcrun --show-sdk-path
  OUTPUT_VARIABLE MACOS_SDK_PATH
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

if(${SLK_TOOLCHAIN_NAME} STREQUAL "clang")
    # Set the compilers to Homebrew LLVM clang/clang++
    set(CMAKE_C_COMPILER /usr/local/bin/clang)
    set(CMAKE_CXX_COMPILER /usr/local/bin/clang++)

    set(COMPILER_COMMON_FLAGS "-isystem /opt/homebrew/opt/llvm/include/c++/v1 -isysroot ${MACOS_SDK_PATH}")
    set(COMPILER_CXX_FLAGS "-stdlib=libc++")
elseif(${SLK_TOOLCHAIN_NAME} STREQUAL "gcc")
    # Set the compilers to Homebrew gcc/g++
    set(CMAKE_C_COMPILER "/opt/homebrew/opt/gcc/bin/gcc-15")
    set(CMAKE_CXX_COMPILER "/opt/homebrew/opt/gcc/bin/g++-15")

    # Explicitely specifying paths to gcc standard library for lsp to find includes because otherwise it would use the ones from macos SDK
    set(COMPILER_COMMON_FLAGS "-isystem /opt/homebrew/opt/gcc/include/c++/15 -isystem /opt/homebrew/opt/gcc/include/c++/15/aarch64-apple-darwin25")
    set(COMPILER_CXX_FLAGS)
else()
    message(FATAL_ERROR "Unsupported compiler: ${SLK_TOOLCHAIN_NAME}")
endif()

# Apply compile flags globally (can be overridden per target)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${COMPILER_COMMON_FLAGS} -Wall")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${COMPILER_COMMON_FLAGS} ${COMPILER_CXX_FLAGS} -Wall")

