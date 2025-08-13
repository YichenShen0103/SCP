# Parser module CMakeLists.txt
cmake_minimum_required(VERSION 3.16)

# Define the parser library
add_library(scp_semant STATIC)

# Add source files
target_sources(scp_semant PRIVATE
        type_checker.cpp
)

# Set include directories
target_include_directories(scp_semant PUBLIC
        ${CMAKE_SOURCE_DIR}/include
)

# Link dependencies
target_link_libraries(scp_semant PUBLIC
        scp_core
        scp_lexer
        scp_parser
)

# Set target properties
set_target_properties(scp_semant PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF
)

# Export the target for parent project
set(SCP_SEMANT_TARGET scp_semant PARENT_SCOPE)
