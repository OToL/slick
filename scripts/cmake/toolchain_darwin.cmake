set(SLICK_TARGET_PLATFORM_ID darwin)
set(SLICK_HOST_PLATFORM_ID darwin)

# When setting this variable to gcc, remove -nostdinc++ from .clangd
set(SLICK_TOOLCHAIN_NAME "clang" CACHE STRING "")

# Locate the macOS SDK path dynamically
execute_process(
  COMMAND xcrun --show-sdk-path
  OUTPUT_VARIABLE MACOS_SDK_PATH
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

#[[ 
LLVM build for modules:
1. Compiler/header version mismatch
Your llvm-project/build initially had clang/clangd but no libcxx built — so toolchain_darwin.cmake was still forcing Homebrew's (mismatched-version) libc++ headers via a hardcoded -isystem. Fixed by reconfiguring llvm-project with -DLLVM_ENABLE_RUNTIMES="libcxx;libcxxabi;libunwind" and rebuilding, so the headers/libs now match the compiler.

2. Self-contained install (avoided clobbering Homebrew)
The build's default CMAKE_INSTALL_PREFIX was /usr/local — same place Homebrew's own clang/clangd live. Reconfigured with -DCMAKE_INSTALL_PREFIX=/Users/emmanuelbeau/Documents/dev/llvm-project/install and ran ninja install there instead, so nothing in your Homebrew setup was touched.

3. libc++.modules.json path layout
This manifest (which tells CMake where std.cppm lives) uses paths relative to an installed layout (../share/libc++/v1/...), not a raw build tree's layout (modules/c++/v1/...). This is exactly why the real ninja install in step 2 was necessary rather than pointing straight at build/.

4. Missing DEFAULT_SYSROOT (the actual "cannot find stdlib.h")
Homebrew's clang has a default macOS SDK sysroot baked in at build time; your manual build didn't, so any compile command lacking an explicit -isysroot (several vendored bgfx/bx CMake targets do) had zero visibility into platform headers at all. Fixed by reconfiguring with -DDEFAULT_SYSROOT=$(xcrun --show-sdk-path) and rebuilding+reinstalling clang+clangd. 

LLVM configure & build commands:
    cmake -G Ninja -S llvm -B build \
      -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_INSTALL_PREFIX=/Users/emmanuelbeau/Documents/dev/llvm-project/install \
      -DLLVM_ENABLE_PROJECTS="clang;clang-tools-extra" \
      -DLLVM_ENABLE_RUNTIMES="libcxx;libcxxabi;libunwind" \
      -DDEFAULT_SYSROOT="$(xcrun --show-sdk-path)" \
      -DLLVM_TARGETS_TO_BUILD=all

    ninja -C build clang clangd
    ninja -C build install

Places to change when switching between llvm/clang versions:
- variables below
- CMakePresets.json CMAKE_CXX_STDLIB_MODULES_JSON 
- clangd_path path in nvim/lua/configs/lsp.lua 
]]
if(${SLICK_TOOLCHAIN_NAME} STREQUAL "clang")
    # /Users/emmanuelbeau/Documents/dev/llvm-project/install/lib/libc++.modules.json
    set(CLANG_BIN_ROOT "/Users/emmanuelbeau/Documents/dev/llvm-project/install/bin")
    set(LLVM_ROOT "/Users/emmanuelbeau/Documents/dev/llvm-project/install/")
    set(LLVM_LIB_PATH ${LLVM_ROOT}/lib)

    #/opt/homebrew/opt/llvm/lib/c++/libc++.modules.json
    # set(CLANG_BIN_ROOT "/usr/local/bin")
    # set(LLVM_ROOT "/opt/homebrew/opt/llvm")
    # set(LLVM_LIB_PATH /opt/homebrew/opt/llvm/lib/c++)

    # Set the compilers to Homebrew LLVM clang/clang++
    set(CMAKE_C_COMPILER ${CLANG_BIN_ROOT}/clang)
    set(CMAKE_CXX_COMPILER ${CLANG_BIN_ROOT}/clang++)
    set(CMAKE_CXX_COMPILER_CLANG_SCAN_DEPS ${CLANG_BIN_ROOT}/clang-scan-deps)

    set(COMPILER_COMMON_FLAGS "-isystem ${LLVM_ROOT}/include/c++/v1 -isysroot ${MACOS_SDK_PATH}")
    set(COMPILER_CXX_FLAGS "-stdlib=libc++ -Wno-reserved-module-identifier")

    # Use homebrew lib c++ because the one from macos is outdated
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -L${LLVM_LIB_PATH} -Wl,-rpath,${LLVM_LIB_PATH}")
elseif(${SLICK_TOOLCHAIN_NAME} STREQUAL "gcc")
    # Set the compilers to Homebrew gcc/g++
    set(CMAKE_C_COMPILER "/opt/homebrew/opt/gcc/bin/gcc-15")
    set(CMAKE_CXX_COMPILER "/opt/homebrew/opt/gcc/bin/g++-15")

    # Explicitely specifying paths to gcc standard library for lsp to find includes because otherwise it would use the ones from macos SDK
    set(COMPILER_COMMON_FLAGS "-isystem /opt/homebrew/opt/gcc/include/c++/15 -isystem /opt/homebrew/opt/gcc/include/c++/15/aarch64-apple-darwin25")
    set(COMPILER_CXX_FLAGS)
else()
    message(FATAL_ERROR "Unsupported compiler: ${SLICK_TOOLCHAIN_NAME}")
endif()

# Apply compile flags globally (can be overridden per target)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${COMPILER_COMMON_FLAGS} -Wall")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${COMPILER_COMMON_FLAGS} ${COMPILER_CXX_FLAGS} -Wall")
