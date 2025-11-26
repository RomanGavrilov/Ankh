# tests/CMakeLists.txt

find_package(GTest REQUIRED)

add_executable(integration_test
    integration_test.cpp
)

target_link_libraries(integration_test
    PRIVATE
        ankh_warnings
        ankh_app
        GTest::gtest
)

target_include_directories(integration_test
    PRIVATE
        ${CMAKE_SOURCE_DIR}/src
)

# Find glslc for shader compilation
find_program(GLSLC_EXECUTABLE glslc)
if(NOT GLSLC_EXECUTABLE)
    message(WARNING "glslc not found - shader compilation may fail")
endif()

# Copy shaders and compile them after build
add_custom_command(
    TARGET integration_test
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_SOURCE_DIR}/shaders"
            "$<TARGET_FILE_DIR:integration_test>/shaders"
    COMMAND ${GLSLC_EXECUTABLE} "$<TARGET_FILE_DIR:integration_test>/shaders/vert.vert" -o "$<TARGET_FILE_DIR:integration_test>/shaders/vert.spv"
    COMMAND ${GLSLC_EXECUTABLE} "$<TARGET_FILE_DIR:integration_test>/shaders/frag.frag" -o "$<TARGET_FILE_DIR:integration_test>/shaders/frag.spv"
    COMMENT "Copying and compiling shaders for integration test"
)

# Add test to CTest
include(GoogleTest)
gtest_discover_tests(integration_test)
