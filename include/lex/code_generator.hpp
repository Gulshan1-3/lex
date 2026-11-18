#ifndef LEX_CODE_GENERATOR_HPP
#define LEX_CODE_GENERATOR_HPP

#include "lex/dfa.hpp"
#include "lex/spec_parser.hpp"
#include <string>

namespace lex {

struct Generator_options {
    std::string class_name{"Lexer"};
    std::string namespace_name{""};
    bool emit_header_only{false};
    bool generate_main_if_missing{true};
};

// Generates modern, idiomatic C++20 scanner code from minimized DFA and specification
class Code_generator {
public:
    Code_generator(const Dfa& dfa, const Spec_file& spec, Generator_options options = {})
        : dfa_{dfa}, spec_{spec}, options_{std::move(options)} {}

    [[nodiscard]] std::string generate_source() const;
    [[nodiscard]] std::string generate_header() const;

private:
    [[nodiscard]] std::string emit_transition_tables() const;
    [[nodiscard]] std::string emit_actions() const;

    const Dfa& dfa_;
    const Spec_file& spec_;
    Generator_options options_;
};

} // namespace lex

#endif // LEX_CODE_GENERATOR_HPP
