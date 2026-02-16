cmake_minimum_required(VERSION 4.0)

project(slick LANGUAGES C CXX)

# Global options
OPTION(SLK_ENABLE_BUILD_EXTERN "Build external dependencies from source" ON)

# Global constants
set(SLK_ROOT_DIR ${CMAKE_CURRENT_LIST_DIR})
set(SLK_SRC_DIR "${CMAKE_CURRENT_LIST_DIR}/src")
set(SLK_SAMPLES_DIR "${CMAKE_CURRENT_LIST_DIR}/samples")
set(SLK_TOOLS_DIR "${CMAKE_CURRENT_LIST_DIR}/tools")

set(SLK_EXTERN_INCLUDE_DIR "${CMAKE_CURRENT_LIST_DIR}/extern/redist/include")
set(SLK_EXTERN_EXTRA_DIR "${CMAKE_CURRENT_LIST_DIR}/extern/redist/extra")
set(SLK_EXTERN_LIB_DIR "${CMAKE_CURRENT_LIST_DIR}/extern/redist/lib")
set(SLK_EXTERN_TARGET_LIB_DIR "${SLK_EXTERN_LIB_DIR}/${SLK_TARGET_PLATFORM_ID}")
set(SLK_EXTERN_HOST_LIB_DIR "${SLK_EXTERN_LIB_DIR}/${SLK_HOST_PLATFORM_ID}")
set(SLK_EXTERN_BIN_DIR "${CMAKE_CURRENT_LIST_DIR}/extern/redist/bin")
set(SLK_EXTERN_TARGET_BIN_DIR "${SLK_EXTERN_BIN_DIR}/${SLK_TARGET_PLATFORM_ID}")
set(SLK_EXTERN_HOST_BIN_DIR "${SLK_EXTERN_BIN_DIR}/${SLK_HOST_PLATFORM_ID}")

include(scripts/cmake/slk_utilities.cmake)

add_subdirectory("extern")
add_subdirectory("src")
add_subdirectory("tools")
add_subdirectory("samples")
