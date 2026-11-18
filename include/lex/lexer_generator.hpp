#ifndef LEX_LEXER_GENERATOR_HPP
#define LEX_LEXER_GENERATOR_HPP

#include "lex/code_generator.hpp"
#include "lex/dfa_minimizer.hpp"
#include "lex/regex.hpp"
#include "lex/spec_parser.hpp"
#include <string>
#include <string_view>

namespace lex {

struct Generator_result {
    std::string source_code{};
    std::string header_code{};
    std::size_t nfa_states{0};
    std::size_t dfa_states{0};
    std::size_t min_dfa_states{0};
    std::string dot_nfa{};
    std::string dot_dfa{};
};

// Orchestrates the complete pipeline: Spec -> Regex AST -> NFA -> DFA -> Min DFA -> C++ Code
class Lexer_generator {
public:
    explicit Lexer_generator(Generator_options options = {}) noexcept
        : options_{std::move(options)} {}

    [[nodiscard]] Generator_result generate(std::string_view spec_content);

private:
    Generator_options options_;
};

} // namespace lex

#endif // LEX_LEXER_GENERATOR_HPP
