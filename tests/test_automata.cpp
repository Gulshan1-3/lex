#include "lex/dfa_minimizer.hpp"
#include "lex/nfa.hpp"
#include "lex/regex.hpp"
#include "test_harness.hpp"

void test_nfa_construction() {
    lex::Nfa nfa;
    lex::Thompson_builder builder{nfa};

    lex::Regex_parser parser("a|b*");
    auto ast = parser.parse();
    auto frag = builder.build(*ast);

    TEST_CHECK(frag.start != lex::invalid_state);
    TEST_CHECK(frag.accept != lex::invalid_state);
    TEST_CHECK(nfa.size() > 0);
}

void test_dfa_subset_and_minimization() {
    lex::Nfa nfa;
    lex::State_id master = nfa.add_state();
    nfa.set_start_state(master);
    lex::Thompson_builder builder{nfa};

    // Rule 0: "if"
    lex::Regex_parser p0("if");
    auto frag0 = builder.build(*p0.parse());
    nfa.add_epsilon_transition(master, frag0.start);
    nfa.set_rule(frag0.accept, 0);

    // Rule 1: [a-z]+
    lex::Regex_parser p1("[a-z]+");
    auto frag1 = builder.build(*p1.parse());
    nfa.add_epsilon_transition(master, frag1.start);
    nfa.set_rule(frag1.accept, 1);

    lex::Dfa_builder dfa_builder{nfa};
    lex::Dfa dfa = dfa_builder.build();

    TEST_CHECK(dfa.size() > 0);

    lex::Dfa_minimizer minimizer{dfa};
    lex::Dfa min_dfa = minimizer.minimize();

    TEST_CHECK(min_dfa.size() > 0);
    TEST_CHECK(min_dfa.size() <= dfa.size());

    // Trace "if": start --'i'--> s1 --'f'--> s2
    auto s1 = min_dfa.transition(min_dfa.start_state(), 'i');
    TEST_CHECK(s1 != lex::invalid_state);
    auto s2 = min_dfa.transition(s1, 'f');
    TEST_CHECK(s2 != lex::invalid_state);
    // At s2, both "if" (rule 0) and "[a-z]+" (rule 1) match, but Rule 0 must win due to priority!
    TEST_CHECK_EQ(min_dfa.state(s2).rule_id, 0);

    // Trace "in": start --'i'--> s1 --'n'--> s3
    auto s3 = min_dfa.transition(s1, 'n');
    TEST_CHECK(s3 != lex::invalid_state);
    TEST_CHECK_EQ(min_dfa.state(s3).rule_id, 1);
}

void run_automata_tests() {
    lex::test::run_test("NFA Construction", test_nfa_construction);
    lex::test::run_test("DFA Subset & Minimization & Priority", test_dfa_subset_and_minimization);
}
