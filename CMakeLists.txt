cmake_minimum_required(VERSION 3.16)
project(Ankh LANGUAGES CXX)


# ---- Options ---------------------------------------------------------------
option(ANKH_WARNINGS_AS_ERRORS "Treat warnings as errors" OFF)
option(ANKH_BUILD_TESTS "Build tests" OFF)


# ---- Language & defaults ---------------------------------------------------
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(NOT CMAKE_BUILD_TYPE)
set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()


# ---- Third‑party (from source) --------------------------------------------
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(libs/glfw) # target: glfw
add_subdirectory(libs/glm) # target: glm


# ---- Vulkan ---------------------------------------------------------------
find_package(Vulkan REQUIRED) # target: Vulkan::Vulkan


# ---- Warnings target -------------------------------------------------------
add_library(ankh_warnings INTERFACE)
if(MSVC)
target_compile_options(ankh_warnings INTERFACE /W4 /permissive-)
else()
target_compile_options(ankh_warnings INTERFACE -Wall -Wextra -Wpedantic)
endif()
if(ANKH_WARNINGS_AS_ERRORS)
if(MSVC)
target_compile_options(ankh_warnings INTERFACE /WX)
else()
target_compile_options(ankh_warnings INTERFACE -Werror)
endif()
endif()


# ---- Project sources -------------------------------------------------------
add_subdirectory(src)


# ---- Shaders (target-based utility) ---------------------------------------
add_custom_target(ankh_compile_shaders
COMMAND ${CMAKE_COMMAND} -P "${CMAKE_SOURCE_DIR}/shaders/compile.cmake"
WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/shaders"
COMMENT "Compiling shaders"
)


add_custom_target(ankh_copy_shaders
COMMAND ${CMAKE_COMMAND} -E copy_directory
"${CMAKE_SOURCE_DIR}/shaders"
"$<TARGET_FILE_DIR:Ankh>/shaders"
COMMENT "Copying shaders to output directory"
)
add_dependencies(ankh_copy_shaders ankh_compile_shaders)
add_dependencies(Ankh ankh_copy_shaders)


# Optional install
install(TARGETS Ankh RUNTIME DESTINATION bin)