# Lexer module CMakeLists.txt
cmake_minimum_required(VERSION 3.16)

# Define the lexer library
add_library(scp_lexer STATIC)

# Add source files
target_sources(scp_lexer PRIVATE
    dfa.cpp
    lexer.cpp
)

# Set include directories
target_include_directories(scp_lexer PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)

# Link dependencies
target_link_libraries(scp_lexer PUBLIC
    scp_core
)

# Set target properties
set_target_properties(scp_lexer PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

# Export the target for parent project
set(SCP_LEXER_TARGET scp_lexer PARENT_SCOPE)
