# Test module CMakeLists.txt
cmake_minimum_required(VERSION 3.16)

# Find GoogleTest
find_package(GTest REQUIRED)
if(NOT GTest_FOUND)
    message(FATAL_ERROR "GoogleTest not found. Please install it: brew install googletest")
endif()

# Function to create GoogleTest test executable
function(create_gtest_executable target_name sources)
    add_executable(${target_name} ${sources})
    set_target_properties(${target_name} PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/test
    )
    target_include_directories(${target_name} PRIVATE ${CMAKE_SOURCE_DIR}/include)
    target_link_libraries(${target_name} scp_parser GTest::gtest_main)
endfunction()

# Create test executables
create_gtest_executable(dfa_test "dfa_test.cpp")
create_gtest_executable(lexer_test "lexer_test.cpp")
create_gtest_executable(parser_test "parser_test.cpp")

# Add tests to CTest
add_test(NAME dfa_test COMMAND dfa_test)
add_test(NAME lexer_test COMMAND lexer_test)
add_test(NAME parser_test COMMAND parser_test)
