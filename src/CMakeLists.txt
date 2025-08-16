# Tools module CMakeLists.txt
cmake_minimum_required(VERSION 3.16)

# Function to create executable with common properties
function(create_bin_executable target_name sources)
    add_executable(${target_name} ${sources})
    set_target_properties(${target_name} PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    )
    target_include_directories(${target_name} PRIVATE ${CMAKE_SOURCE_DIR}/include)
endfunction()

# Main binary
create_bin_executable(lexer "lexer.cpp")
target_link_libraries(lexer scp_lexer)

create_bin_executable(parser "parser.cpp")
target_link_libraries(parser scp_parser)

create_bin_executable(scpc "scpc.cpp")
target_link_libraries(scpc scp_cgen)