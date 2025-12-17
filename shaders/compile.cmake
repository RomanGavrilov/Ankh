# Shader compilation script for CMake
# Compiles GLSL shaders to SPIR-V

if(NOT DEFINED DESTINATION_DIR)
    message(FATAL_ERROR "DESTINATION_DIR must be defined")
endif()

# Find glslc compiler
find_program(GLSLC glslc)
if(NOT GLSLC)
    message(WARNING "glslc not found - shader compilation skipped")
    return()
endif()

# Create destination directory
file(MAKE_DIRECTORY "${DESTINATION_DIR}")

# Get all shader files in the source directory
file(GLOB SHADER_FILES 
    "${CMAKE_CURRENT_LIST_DIR}/*.vert" 
    "${CMAKE_CURRENT_LIST_DIR}/*.frag"
    "${CMAKE_CURRENT_LIST_DIR}/*.comp"
    "${CMAKE_CURRENT_LIST_DIR}/*.geom"
    "${CMAKE_CURRENT_LIST_DIR}/*.tesc"
    "${CMAKE_CURRENT_LIST_DIR}/*.tese"
)

# Compile each shader
foreach(SHADER ${SHADER_FILES})
    get_filename_component(SHADER_NAME ${SHADER} NAME)
    set(SPIRV "${DESTINATION_DIR}/${SHADER_NAME}.spv")
    
    message(STATUS "Compiling shader: ${SHADER_NAME}")
    execute_process(
        COMMAND ${GLSLC} ${SHADER} -o ${SPIRV}
        RESULT_VARIABLE RESULT
        ERROR_VARIABLE ERROR_MSG
    )
    
    if(NOT RESULT EQUAL 0)
        message(WARNING "Failed to compile ${SHADER_NAME}: ${ERROR_MSG}")
    endif()
endforeach()

message(STATUS "Shader compilation complete")
