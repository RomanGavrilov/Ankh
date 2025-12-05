cmake_minimum_required(VERSION 3.16)
project(Ankh LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(libs/glfw)
add_subdirectory(libs/glm)

find_package(Vulkan REQUIRED)

add_library(ankh_warnings INTERFACE)
if(MSVC)
    target_compile_options(ankh_warnings INTERFACE /W4 /permissive-)
else()
    target_compile_options(ankh_warnings INTERFACE -Wall -Wextra -Wpedantic)
endif()

add_subdirectory(src)
