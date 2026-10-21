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

# Semantic analyzer targets
add_library(storylang_semantic STATIC analyzer.h symbol_table.h analyzer.cpp symbol_table.cpp)
target_include_directories(storylang_semantic PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(storylang_semantic PUBLIC storylang_core)

add_executable(analyzer_test analyzer_test.cpp)
target_include_directories(analyzer_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(analyzer_test storylang_semantic storylang_core)

# Intermediate Code Generator (3AC), Optimizer & Target Code Generator targets
add_library(storylang_ir STATIC 
    ir.h ir.cpp 
    codegen.h codegen.cpp
    optimizer.h optimizer.cpp
    target_codegen.h target_codegen.cpp
)
target_include_directories(storylang_ir PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(storylang_ir PUBLIC storylang_core)

add_executable(ir_test ir_test.cpp)
target_include_directories(ir_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(ir_test storylang_ir storylang_core)

add_executable(optimizer_test optimizer_test.cpp)
target_include_directories(optimizer_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(optimizer_test storylang_ir storylang_core)

add_executable(target_codegen_test target_codegen_test.cpp)
target_include_directories(target_codegen_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(target_codegen_test storylang_ir storylang_core)

# Enable testing
enable_testing()
add_test(NAME LexerTest COMMAND lexer_test)
add_test(NAME ParserTest COMMAND parser_test)
add_test(NAME AnalyzerTest COMMAND analyzer_test)
add_test(NAME IRTest COMMAND ir_test)
add_test(NAME OptimizerTest COMMAND optimizer_test)
add_test(NAME TargetCodegenTest COMMAND target_codegen_test)

# CLI executable
add_executable(storylang_cli cli.cpp)
target_link_libraries(storylang_cli storylang_ir storylang_semantic storylang_core)

# Print configuration
message(STATUS "=== StoryLang Compiler Configuration ===")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER}")