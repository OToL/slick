set(SLK_TARGET_PLATFORM_ID darwin)

# Set the compilers to Homebrew LLVM clang/clang++
set(CMAKE_C_COMPILER /usr/local/bin/clang)
set(CMAKE_CXX_COMPILER /usr/local/bin/clang++)

# Set C++ standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Locate the macOS SDK path dynamically
execute_process(
  COMMAND xcrun --show-sdk-path
  OUTPUT_VARIABLE MACOS_SDK_PATH
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

# Common compile flags for Homebrew LLVM on macOS
set(COMMON_FLAGS "-isystem /opt/homebrew/opt/llvm/include/c++/v1 -isysroot ${MACOS_SDK_PATH} -Wall")

# Apply compile flags globally (can be overridden per target)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${COMMON_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${COMMON_FLAGS} -stdlib=libc++")

# Linker flags for libc++ and macOS SDK
# set(CMAKE_EXE_LINKER_FLAGS
#   "-L/opt/homebrew/opt/llvm/lib -Wl,-rpath,/opt/homebrew/opt/llvm/lib -isysroot ${MACOS_SDK_PATH}"
# )

