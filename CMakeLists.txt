cmake_minimum_required(VERSION 3.20)
project(lex_cpp VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Founder of C++ style: strict warnings to ensure clean, standard-compliant code
if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion -Wshadow)
endif()

include_directories(include)

# Core library
add_library(lex_core STATIC
    src/char_set.cpp
    src/regex.cpp
    src/nfa.cpp
    src/dfa.cpp
    src/dfa_minimizer.cpp
    src/spec_parser.cpp
    src/code_generator.cpp
    src/lexer_generator.cpp
)

target_include_directories(lex_core PUBLIC include)

# Main executable (lex_cpp compiler tool)
add_executable(lex_cpp src/main.cpp)
target_link_libraries(lex_cpp PRIVATE lex_core)

# Tests
enable_testing()

add_executable(test_runner
    tests/test_char_set.cpp
    tests/test_regex.cpp
    tests/test_automata.cpp
    tests/test_end_to_end.cpp
)
target_link_libraries(test_runner PRIVATE lex_core)

add_test(NAME AllUnitAndIntegrationTests COMMAND test_runner)
