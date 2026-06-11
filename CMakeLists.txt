cmake_minimum_required(VERSION 4.0)

project(slick LANGUAGES C CXX)

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
add_subdirectory("tools")
add_subdirectory("samples")
