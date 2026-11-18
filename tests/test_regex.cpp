#include "lex/regex.hpp"
#include "test_harness.hpp"

void test_regex_literals_and_escapes() {
    lex::Regex_parser parser1("abc");
    auto ast1 = parser1.parse();
    TEST_CHECK(ast1 != nullptr);
    TEST_CHECK(ast1->kind() == lex::Regex_kind::Concat);

    lex::Regex_parser parser2(R"(\n\t\x41)");
    auto ast2 = parser2.parse();
    TEST_CHECK(ast2 != nullptr);
    TEST_CHECK(ast2->kind() == lex::Regex_kind::Concat);

    lex::Regex_parser parser3(R"("int"|"float")");
    auto ast3 = parser3.parse();
    TEST_CHECK(ast3 != nullptr);
    TEST_CHECK(ast3->kind() == lex::Regex_kind::Union);
}

void test_regex_classes_and_operators() {
    lex::Regex_parser parser1("[a-zA-Z_][a-zA-Z0-9_]*");
    auto ast1 = parser1.parse();
    TEST_CHECK(ast1 != nullptr);
    TEST_CHECK(ast1->kind() == lex::Regex_kind::Concat);

    lex::Regex_parser parser2("[^0-9]+");
    auto ast2 = parser2.parse();
    TEST_CHECK(ast2 != nullptr);
    TEST_CHECK(ast2->kind() == lex::Regex_kind::Repeat);

    lex::Regex_parser parser3("a?b+c*");
    auto ast3 = parser3.parse();
    TEST_CHECK(ast3 != nullptr);
    TEST_CHECK(ast3->kind() == lex::Regex_kind::Concat);
}

void test_regex_macro_expansion() {
    lex::Regex_parser::Macro_table macros;
    macros["DIGIT"] = "[0-9]";
    macros["HEX"] = "[0-9a-fA-F]";

    lex::Regex_parser parser("0x{HEX}+|{DIGIT}+", macros);
    auto ast = parser.parse();
    TEST_CHECK(ast != nullptr);
    TEST_CHECK(ast->kind() == lex::Regex_kind::Union);
}

void run_regex_tests() {
    lex::test::run_test("Regex Literals & Escapes", test_regex_literals_and_escapes);
    lex::test::run_test("Regex Classes & Operators", test_regex_classes_and_operators);
    lex::test::run_test("Regex Macro Expansion", test_regex_macro_expansion);
}
