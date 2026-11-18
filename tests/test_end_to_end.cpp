#include "lex/lexer_generator.hpp"
#include "lex/spec_parser.hpp"
#include "test_harness.hpp"
#include <fstream>
#include <sstream>

extern void run_char_set_tests();
extern void run_regex_tests();
extern void run_automata_tests();

void test_spec_parser() {
    std::string spec_src = R"(
%{
#include <iostream>
%}

DIGIT  [0-9]
ID     [a-zA-Z_][a-zA-Z0-9_]*

%%

{DIGIT}+        { return 100; }
{ID}            { return 101; }
"+"             { return 102; }
[ \t\n]+        { /* skip */ }
.               { return 999; }

%%

int dummy() { return 42; }
)";

    lex::Spec_parser parser{spec_src};
    lex::Spec_file spec = parser.parse();

    TEST_CHECK(spec.prologue.find("#include <iostream>") != std::string::npos);
    TEST_CHECK_EQ(spec.definitions.size(), 2);
    TEST_CHECK_EQ(spec.definitions["DIGIT"], "[0-9]");
    TEST_CHECK_EQ(spec.rules.size(), 5);
    TEST_CHECK(spec.epilogue.find("int dummy()") != std::string::npos);
}

void test_full_pipeline_generation() {
    std::string spec_src = R"(
%{
#include <string>
%}

DIGIT   [0-9]
HEX     [0-9a-fA-F]

%%

"0x"{HEX}+     { return 1; }
{DIGIT}+       { return 2; }
">="           { return 3; }
">>"           { return 4; }
">"            { return 5; }
[ \t\r\n]+     { /* skip whitespace */ }
.              { return -1; }

%%
)";

    lex::Lexer_generator generator;
    auto result = generator.generate(spec_src);

    TEST_CHECK(!result.source_code.empty());
    TEST_CHECK(result.nfa_states > 0);
    TEST_CHECK(result.dfa_states > 0);
    TEST_CHECK(result.min_dfa_states > 0);
    TEST_CHECK(result.min_dfa_states <= result.dfa_states);
    TEST_CHECK(result.source_code.find("class Lexer") != std::string::npos);
    TEST_CHECK(result.source_code.find("dfa_transitions") != std::string::npos);
}

void run_end_to_end_tests() {
    lex::test::run_test("Spec Parser", test_spec_parser);
    lex::test::run_test("Full Pipeline Generation", test_full_pipeline_generation);
}

int main() {
    std::cout << "========================================\n";
    std::cout << " Running lex_cpp Test Suite (Modern C++)\n";
    std::cout << "========================================\n";

    run_char_set_tests();
    run_regex_tests();
    run_automata_tests();
    run_end_to_end_tests();

    std::cout << "========================================\n";
    std::cout << " Total tests:    " << lex::test::g_test_count << "\n";
    std::cout << " Total failures: " << lex::test::g_test_failures << "\n";
    std::cout << "========================================\n";

    return (lex::test::g_test_failures == 0) ? 0 : 1;
}
