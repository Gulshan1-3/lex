#include "lex/lexer_generator.hpp"
#include <stdexcept>

namespace lex {

Generator_result Lexer_generator::generate(std::string_view spec_content) {
    // Step 1: Parse the Lex specification
    Spec_parser spec_parser{spec_content};
    Spec_file spec = spec_parser.parse();

    if (spec.rules.empty()) {
        throw std::runtime_error("Lex specification contains no rules in section 2");
    }

    // Step 2: Build combined NFA with rule priorities
    Nfa nfa;
    State_id master_start = nfa.add_state();
    nfa.set_start_state(master_start);
    Thompson_builder builder{nfa};

    for (const auto& rule : spec.rules) {
        Regex_parser regex_parser{rule.pattern, spec.definitions};
        auto ast = regex_parser.parse();
        auto frag = builder.build(*ast);
        nfa.add_epsilon_transition(master_start, frag.start);
        nfa.set_rule(frag.accept, rule.id);
    }

    // Step 3: Subset construction -> DFA
    Dfa_builder dfa_builder{nfa};
    Dfa raw_dfa = dfa_builder.build();

    // Step 4: DFA Minimization (Hopcroft / Moore partition refinement)
    Dfa_minimizer minimizer{raw_dfa};
    Dfa min_dfa = minimizer.minimize();

    // Step 5: Code Generation
    Code_generator code_gen{min_dfa, spec, options_};

    Generator_result result;
    result.source_code = code_gen.generate_source();
    result.header_code = code_gen.generate_header();
    result.nfa_states = nfa.size();
    result.dfa_states = raw_dfa.size();
    result.min_dfa_states = min_dfa.size();
    result.dot_nfa = nfa.to_dot();
    result.dot_dfa = min_dfa.to_dot();

    return result;
}

} // namespace lex
