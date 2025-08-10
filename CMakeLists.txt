# CMake minimum version requirement
cmake_minimum_required(VERSION 3.16)

# Project name and version
project(SCP VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard globally
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Set build type to Debug if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

# Compiler flags
set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Create output directories
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/test)
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Set output directories
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")  
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")  
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")  

# Add subdirectories for each module
# Note: Order matters due to dependencies (constant -> core -> lexer -> parser)
add_subdirectory(src/core)
add_subdirectory(src/lexer)
add_subdirectory(src/parser)
add_subdirectory(src/)

# Enable testing and add test subdirectory
enable_testing()
add_subdirectory(test)

# Installation rules
install(TARGETS lexer parser
    RUNTIME DESTINATION bin
)

install(TARGETS scp_core scp_lexer scp_parser
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

install(DIRECTORY include/
    DESTINATION include
    FILES_MATCHING PATTERN "*.h"
)

# Development tools configuration
# This file contains targets for code formatting, linting, and static analysis

# Find development tools
find_program(CLANG_FORMAT_BIN
    NAMES clang-format clang-format-12 clang-format-14
)

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

find_program(CPPLINT_BIN
    NAMES cpplint cpplint.py
)

if ("${CPPLINT_BIN}" STREQUAL "CPPLINT_BIN-NOTFOUND")
    message(WARNING "SCP couldn't find cpplint.")
else ()
    message(STATUS "SCP found cpplint at ${CPPLINT_BIN}")
endif ()

# Collect all source files
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

# Code formatting target
add_custom_target(format 
    ${CMAKE_SOURCE_DIR}/build_support/run_clang_format.py
    ${CLANG_FORMAT_BIN}
    ${CMAKE_SOURCE_DIR}/build_support/clang_format_exclusions.txt
    --source_dirs
    ${FORMAT_DIRS}
    --fix
    --quiet
)

# Linting target
add_custom_target(check-lint 
    echo '${ALL_SOURCE_FILES}' | xargs -n12 -P8
    ${CPPLINT_BIN}
    --verbose=2 --quiet
    --linelength=120
    --filter=-legal/copyright,-build/header_guard,-runtime/references,-whitespace/indent_namespace
)

# Static analysis target
add_custom_target(check-clang-tidy
    ${CMAKE_SOURCE_DIR}/build_support/run_clang_tidy.py
    -clang-tidy-binary ${CLANG_TIDY_BIN}
    -p ${CMAKE_BINARY_DIR}
    -header-filter=${CMAKE_SOURCE_DIR}/include/.*
    ${ALL_SOURCE_FILES}
)
