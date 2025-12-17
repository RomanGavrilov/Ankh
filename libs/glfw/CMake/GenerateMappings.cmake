# Simple script to copy mappings.h.in to mappings.h
# This is a simplified version for build purposes

if(CMAKE_ARGC LESS 4)
    message(FATAL_ERROR "Usage: cmake -P GenerateMappings.cmake <input> <output>")
endif()

set(INPUT_FILE "${CMAKE_ARGV3}")
set(OUTPUT_FILE "${CMAKE_ARGV4}")

if(NOT EXISTS "${INPUT_FILE}")
    message(FATAL_ERROR "Input file does not exist: ${INPUT_FILE}")
endif()

# For now, just copy the file as-is
configure_file("${INPUT_FILE}" "${OUTPUT_FILE}" COPYONLY)
message(STATUS "Generated ${OUTPUT_FILE} from ${INPUT_FILE}")
