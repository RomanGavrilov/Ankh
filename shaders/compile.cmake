# Shader compilation script for CMake
cmake_minimum_required(VERSION 3.16)

# Find glslc
find_program(GLSLC_EXECUTABLE glslc)
if(NOT GLSLC_EXECUTABLE)
    message(FATAL_ERROR "glslc not found")
endif()

# Get the directory where this script is located
get_filename_component(SHADER_DIR ${CMAKE_CURRENT_LIST_FILE} PATH)

# Compile shaders
execute_process(
    COMMAND ${GLSLC_EXECUTABLE} ${SHADER_DIR}/vert.vert -o ${SHADER_DIR}/vert.spv
    RESULT_VARIABLE RESULT
)
if(NOT RESULT EQUAL 0)
    message(FATAL_ERROR "Failed to compile vert.vert")
endif()

execute_process(
    COMMAND ${GLSLC_EXECUTABLE} ${SHADER_DIR}/frag.frag -o ${SHADER_DIR}/frag.spv
    RESULT_VARIABLE RESULT
)
if(NOT RESULT EQUAL 0)
    message(FATAL_ERROR "Failed to compile frag.frag")
endif()

message(STATUS "Shaders compiled successfully")
