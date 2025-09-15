cmake_minimum_required(VERSION 4.0)

project(slick LANGUAGES C CXX)

# Global options
OPTION(SLK_ENABLE_BUILD_EXTERN "Build external dependencies from source" ON)

# Global constants
set(SLK_ROOT_DIR ${CMAKE_CURRENT_LIST_DIR})
set(SLK_SRC_DIR "${CMAKE_CURRENT_LIST_DIR}/slk/src")
set(SLK_EXTERN_INC_DIR "${CMAKE_CURRENT_LIST_DIR}/extern/redis/inc")

include(scripts/make/slk_toolchain.cmake)

add_subdirectory("extern")
add_subdirectory("tools")
