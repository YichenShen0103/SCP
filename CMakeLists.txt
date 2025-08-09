# CMake minimum version requirement
cmake_minimum_required(VERSION 3.16)

# Project name and version
project(SCP VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Find and include GoogleTest
find_package(GTest REQUIRED)
if(GTest_FOUND)
    message(STATUS "GoogleTest found")
else()
    message(FATAL_ERROR "GoogleTest not found. Please install it: brew install googletest")
endif()

# Set build type to Debug if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

# Compiler flags
set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)

# Function to create executable with common properties
function(create_executable target_name sources)
    add_executable(${target_name} ${sources})
    set_target_properties(${target_name} PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
    )
    target_include_directories(${target_name} PRIVATE ${CMAKE_SOURCE_DIR}/include)
endfunction()

# Function to create GoogleTest test executable
function(create_gtest_executable target_name sources)
    add_executable(${target_name} ${sources})
    set_target_properties(${target_name} PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/test
    )
    target_include_directories(${target_name} PRIVATE ${CMAKE_SOURCE_DIR}/include)
    target_link_libraries(${target_name} scp_parser GTest::gtest_main)
endfunction()

# Function to create test executable
function(create_test_executable target_name sources output_dir)
    create_executable(${target_name} "${sources}")
    set_target_properties(${target_name} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${output_dir}
    )
    target_link_libraries(${target_name} scp_parser)
endfunction()

# Create directories
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/test)
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")  
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")  
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")  

# Core library (tokens, AST)
add_library(scp_core STATIC)
target_sources(scp_core PRIVATE
    src/core/token.cpp
)
target_include_directories(scp_core PUBLIC ${CMAKE_SOURCE_DIR}/include)
set_target_properties(scp_core PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)

# Lexer library (DFA, Lexer)
add_library(scp_lexer STATIC
    src/lexer/dfa.cpp
    src/lexer/lexer.cpp
)
target_include_directories(scp_lexer PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(scp_lexer scp_core)
set_target_properties(scp_lexer PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)

# Parser library (LL1 Parser)
add_library(scp_parser STATIC
    src/parser/ll1_parser.cpp
)
target_include_directories(scp_parser PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(scp_parser scp_lexer scp_core)
set_target_properties(scp_parser PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)

# Main lexer tool
create_executable(lexer "src/tools/lexer.cpp")
set_target_properties(lexer PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
target_link_libraries(lexer scp_lexer)

# GoogleTest test executables
create_gtest_executable(dfa_test "test/dfa_test.cpp")
create_gtest_executable(lexer_test "test/lexer_test.cpp")

# Installation rules
install(TARGETS lexer
    RUNTIME DESTINATION bin
)

install(TARGETS scp_core scp_lexer scp_parser
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

install(DIRECTORY include/scp
    DESTINATION include
    FILES_MATCHING PATTERN "*.h"
)

# Enable testing
enable_testing()

# Add GoogleTest tests
add_test(NAME dfa_test COMMAND dfa_test)
add_test(NAME lexer_test COMMAND lexer_test)

# Custom targets
add_custom_target(clean-all
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target clean
    COMMENT "Cleaning all build files"
)

add_custom_target(run-tests
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
    DEPENDS dfa_test lexer_test
    COMMENT "Running all tests"
)

# Code formatting targets
# Usage:
#   make format       - Format all C++ source files
#   make check-format - Check if all files are properly formatted (CI-friendly)
#   make format-diff  - Format only files modified in git working directory
if ("${CLANG_FORMAT_BIN}" STREQUAL "CLANG_FORMAT_BIN-NOTFOUND")
    message(WARNING "SCP couldn't find clang-format.")
else ()
    message(STATUS "SCP found clang-format at ${CLANG_FORMAT_BIN}")
endif ()

find_program(CLANG_TIDY_BIN
        NAMES clang-tidy clang-tidy-12
        )

if ("${CLANG_TIDY_BIN}" STREQUAL "CLANG_TIDY_BIN-NOTFOUND")
    message(WARNING "SCP couldn't find clang-tidy.")
else ()
    # Output compile_commands.json
    set(CMAKE_EXPORT_COMPILE_COMMANDS 1)
    message(STATUS "SCP found clang-tidy at ${CLANG_TIDY_BIN}")
endif ()

# cpplint
find_program(CPPLINT_BIN
        NAMES cpplint cpplint.py
        )

if ("${CPPLINT_BIN}" STREQUAL "CPPLINT_BIN-NOTFOUND")
    message(WARNING "SCP couldn't find cpplint.")
else ()
    message(STATUS "SCP found cpplint at ${CPPLINT_BIN}")
endif ()


file(GLOB_RECURSE ALL_SOURCE_FILES
    ${CMAKE_SOURCE_DIR}/src/*.cpp
    ${CMAKE_SOURCE_DIR}/include/*.h
    ${CMAKE_SOURCE_DIR}/test/*.cpp
    ${CMAKE_SOURCE_DIR}/test/*.h
)

string(CONCAT FORMAT_DIRS
        "${CMAKE_SOURCE_DIR}/src,"
        "${CMAKE_SOURCE_DIR}/include,"
        "${CMAKE_SOURCE_DIR}/test,"
        )


add_custom_target(format ${CMAKE_SOURCE_DIR}/build_support/run_clang_format.py
        ${CLANG_FORMAT_BIN}
        ${CMAKE_SOURCE_DIR}/build_support/clang_format_exclusions.txt
        --source_dirs
        ${FORMAT_DIRS}
        --fix
        --quiet
        )

add_custom_target(check-lint echo '${ALL_SOURCE_FILES}' | xargs -n12 -P8
        ${CPPLINT_BIN}
        --verbose=2 --quiet
        --linelength=120
        --filter=-legal/copyright,-build/header_guard,-runtime/references # https://github.com/cpplint/cpplint/issues/148
        )

add_custom_target(check-clang-tidy
        ${CMAKE_SOURCE_DIR}/build_support/run_clang_tidy.py               # run LLVM's clang-tidy script
        -clang-tidy-binary ${CLANG_TIDY_BIN}                              # using our clang-tidy binary
        -p ${CMAKE_BINARY_DIR}                                            # using cmake's generated compile commands
        -header-filter=${CMAKE_SOURCE_DIR}/include/.*                     # check header files in include directory
        ${ALL_SOURCE_FILES}                                               # all source files
        )
