#ifndef LEX_DFA_MINIMIZER_HPP
#define LEX_DFA_MINIMIZER_HPP

#include "lex/dfa.hpp"

namespace lex {

// Minimizes a DFA using partition refinement (Hopcroft / Moore), preserving rule actions
class Dfa_minimizer {
public:
    explicit Dfa_minimizer(const Dfa& dfa) noexcept : dfa_{dfa} {}

    [[nodiscard]] Dfa minimize();

private:
    const Dfa& dfa_;
};

} // namespace lex

#endif // LEX_DFA_MINIMIZER_HPP
