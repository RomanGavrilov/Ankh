cmake_minimum_required(VERSION 3.12)
project(Ankh)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Debug)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

add_subdirectory(libs/glfw)
add_subdirectory(libs/glm)

find_package(Vulkan REQUIRED)

include_directories(${CMAKE_SOURCE_DIR}/libs/glfw/include)
include_directories(${CMAKE_SOURCE_DIR}/libs/glm)
include_directories(${CMAKE_SOURCE_DIR}/libs/vulkan/include)
include_directories(${CMAKE_SOURCE_DIR}/src)
include_directories(${CMAKE_SOURCE_DIR}/src/scene)
include_directories(${CMAKE_SOURCE_DIR}/src/input)

add_executable(
    Ankh
    ${CMAKE_SOURCE_DIR}/src/main.cpp
)

add_custom_command(
    TARGET Ankh
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/shaders"
        "$<TARGET_FILE_DIR:Ankh>/shaders"
    COMMENT "Copying shaders to output directory"
)

add_custom_command(
    TARGET Ankh
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -P "${CMAKE_SOURCE_DIR}/shaders/compile.cmake"
    VERBATIM
    COMMENT "Compile shaders"
)

target_link_libraries(Ankh glfw)
target_link_libraries(Ankh glm)
target_link_libraries(Ankh Vulkan::Vulkan)