# Parser module CMakeLists.txt
cmake_minimum_required(VERSION 3.16)

# Define the parser library
add_library(scp_parser STATIC)

# Add source files
target_sources(scp_parser PRIVATE
    ll1_parser.cpp
    slr_parser.cpp
)

# Set include directories
target_include_directories(scp_parser PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)

# Link dependencies
target_link_libraries(scp_parser PUBLIC
    scp_lexer
    scp_core
)

# Set target properties
set_target_properties(scp_parser PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

# Export the target for parent project
set(SCP_PARSER_TARGET scp_parser PARENT_SCOPE)
