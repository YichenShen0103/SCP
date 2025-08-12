# Core module CMakeLists.txt
cmake_minimum_required(VERSION 3.16)

# Define the core library
add_library(scp_core STATIC)

# Add source files
target_sources(scp_core PRIVATE
    token.cpp
        symbol_table.cpp
)

# Set include directories
target_include_directories(scp_core PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)

# Set target properties
set_target_properties(scp_core PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

# Export the target for parent project
set(SCP_CORE_TARGET scp_core PARENT_SCOPE)
