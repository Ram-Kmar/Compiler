cmake_minimum_required(VERSION 3.10)
project(compiler)

set(CMAKE_CXX_STANDARD 17)

# Tell CMake to look for header files in the 'src' directory
include_directories(src)

# Define the executable with the new paths
add_executable(compiler 
    src/main.cpp 
    src/lexer.cpp 
    src/parser.cpp
    src/generation.cpp
    src/semantic_analysis.cpp
)