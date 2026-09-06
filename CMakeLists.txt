cmake_minimum_required(VERSION 4.0)

# Has to be before `project` CMake macro to be visible to sub-processes.
# Cannot be in toolchain file becuase it would apply to internal CMake targets such as compiler detection tests.
# if(SLICK_TOOLCHAIN_NAME STREQUAL "clang")
#     set(CMAKE_CXX_MODULE_STD ON CACHE BOOL "")
#     set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "f35a9ac6-8463-4d38-8eec-5d6008153e7d" CACHE STRING "")
#     set(CMAKE_CXX_STDLIB_MODULES_JSON /opt/homebrew/opt/llvm/lib/c++/libc++.modules.json CACHE STRING "")
# endif()

project(slick LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 26)

# Global options
OPTION(SLICK_ENABLE_BUILD_EXTERN "Build external dependencies from source" ON)

# Global constants
set(SLICK_ROOT_DIR ${CMAKE_CURRENT_LIST_DIR})
set(SLICK_SRC_DIR "${CMAKE_CURRENT_LIST_DIR}/src")
set(SLICK_SAMPLES_DIR "${CMAKE_CURRENT_LIST_DIR}/samples")
set(SLICK_TOOLS_DIR "${CMAKE_CURRENT_LIST_DIR}/tools")

set(SLICK_EXTERN_INCLUDE_DIR "${CMAKE_CURRENT_LIST_DIR}/extern/redist/include")
set(SLICK_EXTERN_EXTRA_DIR "${CMAKE_CURRENT_LIST_DIR}/extern/redist/extra")
set(SLICK_EXTERN_LIB_DIR "${CMAKE_CURRENT_LIST_DIR}/extern/redist/lib")
set(SLICK_EXTERN_TARGET_LIB_DIR "${SLICK_EXTERN_LIB_DIR}/${SLICK_TARGET_PLATFORM_ID}")
set(SLICK_EXTERN_HOST_LIB_DIR "${SLICK_EXTERN_LIB_DIR}/${SLICK_HOST_PLATFORM_ID}")
set(SLICK_EXTERN_BIN_DIR "${CMAKE_CURRENT_LIST_DIR}/extern/redist/bin")
set(SLICK_EXTERN_TARGET_BIN_DIR "${SLICK_EXTERN_BIN_DIR}/${SLICK_TARGET_PLATFORM_ID}")
set(SLICK_EXTERN_HOST_BIN_DIR "${SLICK_EXTERN_BIN_DIR}/${SLICK_HOST_PLATFORM_ID}")

include(scripts/cmake/slk_utilities.cmake)

add_subdirectory("extern")
add_subdirectory("src")
add_subdirectory("samples")
