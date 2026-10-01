cmake_minimum_required(VERSION 3.15)
project(StoryLangCompiler VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Source files
set(COMPILER_SOURCES
    lexer.cpp
    ast.cpp
    parser.cpp
)

set(COMPILER_HEADERS
    token.h
    ast.h
    parser.h
)

# Core library
add_library(storylang_core STATIC ${COMPILER_SOURCES} ${COMPILER_HEADERS})
target_include_directories(storylang_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# Test executables
add_executable(lexer_test lexer_test.cpp)
target_include_directories(lexer_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(lexer_test storylang_core)

add_executable(parser_test parser_test.cpp)
target_include_directories(parser_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(parser_test storylang_core)

# Parser targets
add_library(storylang_core STATIC ast.h parser.h parser.cpp)
target_include_directories(storylang_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(parser_test parser_test.cpp)
target_include_directories(parser_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(parser_test storylang_core storylang_lexer)

# Enable testing
enable_testing()
add_test(NAME LexerTest COMMAND lexer_test)
add_test(NAME ParserTest COMMAND parser_test)

# CLI executable (optional, for testing)
add_executable(storylang_cli cli.cpp)
target_link_libraries(storylang_cli storylang_core)

# Print configuration
message(STATUS "=== StoryLang Compiler Configuration ===")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER}")