# tests/CMakeLists.txt

find_package(GTest REQUIRED)

add_executable(integration_test
    integration_test.cpp
)

target_link_libraries(integration_test
    PRIVATE
        GTest::gtest
        pthread
)

# Ensure main Ankh executable is built before running tests
add_dependencies(integration_test Ankh)

# Add test to CTest
include(GoogleTest)
gtest_discover_tests(integration_test
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/tests
)
