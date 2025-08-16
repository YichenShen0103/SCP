# Core module CMakeLists.txt
cmake_minimum_required(VERSION 3.16)

# Define the core library
add_library(scp_cgen STATIC)

# Add source files
target_sources(scp_cgen PRIVATE
        code_generator.cpp
        runtime_environment.cpp
)

# Set include directories
target_include_directories(scp_cgen PUBLIC
        ${CMAKE_SOURCE_DIR}/include
)

# Link dependencies
target_link_libraries(scp_cgen PUBLIC
        scp_lexer
        scp_core
        scp_parser
        scp_semant
)

# Set target properties
set_target_properties(scp_cgen PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF
)

# Export the target for parent project
set(SCP_CORE_TARGET scp_cgen PARENT_SCOPE)
