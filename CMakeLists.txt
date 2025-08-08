# CMake minimum version requirement
cmake_minimum_required(VERSION 3.16)

# Project name and version
project(lexer VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
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

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)

# Source files
set(SOURCES
    src/dfa.cc
    src/lexer.cc
    test/dfa_test.cpp
)

# Extended test source files
set(EXTENDED_TEST_SOURCES
    src/dfa.cc
    src/lexer.cc
    test/lexer_extended_test.cpp
)

# Stream test source files
set(STREAM_TEST_SOURCES
    src/dfa.cc
    src/lexer.cc
    test/lexer_stream_test.cpp
)

# Parser demo source files
set(PARSER_DEMO_SOURCES
    src/dfa.cc
    src/lexer.cc
    test/parser_demo.cpp
)

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES})

# Target properties
set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)

# Optional: Create a library from DFA implementation
add_library(dfa_lib STATIC src/dfa.cc)
target_include_directories(dfa_lib PUBLIC ${CMAKE_SOURCE_DIR}/include)
set_target_properties(dfa_lib PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)

# Link the library to the main executable
target_link_libraries(${PROJECT_NAME} dfa_lib)

# Create extended test executable
add_executable(lexer_extended_test ${EXTENDED_TEST_SOURCES})
set_target_properties(lexer_extended_test PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)
target_link_libraries(lexer_extended_test dfa_lib)

# Create stream test executable
add_executable(lexer_stream_test ${STREAM_TEST_SOURCES})
set_target_properties(lexer_stream_test PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)
target_link_libraries(lexer_stream_test dfa_lib)

# Installation rules (optional)
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
)

install(TARGETS dfa_lib
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

install(FILES include/dfa.h
    DESTINATION include
)

# Enable testing (optional)
enable_testing()

# Add custom target for cleaning
add_custom_target(clean-all
    COMMAND ${CMAKE_COMMAND} -P clean-all.cmake
)