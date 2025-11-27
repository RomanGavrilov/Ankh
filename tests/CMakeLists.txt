# tests/CMakeLists.txt

# Find Python for running pytest
find_package(Python3 REQUIRED COMPONENTS Interpreter)

# Add Python integration tests to CTest
add_test(
    NAME integration_tests
    COMMAND ${Python3_EXECUTABLE} -m pytest ${CMAKE_CURRENT_SOURCE_DIR}/test_integration.py -v
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/src
)

# Set up fixtures to ensure Ankh is built before tests run
set_tests_properties(integration_tests PROPERTIES
    FIXTURES_REQUIRED ankh_build
)

# Create a custom target to mark Ankh as a build fixture
add_custom_target(ankh_test_fixture
    DEPENDS Ankh
)

# Add a test that builds Ankh (used as a fixture setup)
add_test(
    NAME build_ankh
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target Ankh
)

set_tests_properties(build_ankh PROPERTIES
    FIXTURES_SETUP ankh_build
)
