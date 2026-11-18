#ifndef LEX_DFA_HPP
#define LEX_DFA_HPP

#include "lex/char_set.hpp"
#include "lex/nfa.hpp"
#include <array>
#include <cstddef>
#include <string>
#include <vector>

namespace lex {

struct Dfa_state {
    State_id id{invalid_state};
    Rule_id rule_id{no_rule};
    std::array<State_id, Char_set::alphabet_size> transitions{};

    Dfa_state() {
        transitions.fill(invalid_state);
    }

    [[nodiscard]] bool is_accepting() const noexcept { return rule_id != no_rule; }
};

class Dfa {
public:
    Dfa() = default;

    State_id add_state(Rule_id rule = no_rule) {
        State_id id = states_.size();
        Dfa_state st;
        st.id = id;
        st.rule_id = rule;
        states_.push_back(st);
        return id;
    }

    void set_transition(State_id from, unsigned char c, State_id to) noexcept {
        states_[from].transitions[c] = to;
    }

    [[nodiscard]] State_id transition(State_id from, unsigned char c) const noexcept {
        return states_[from].transitions[c];
    }

    [[nodiscard]] const std::vector<Dfa_state>& states() const noexcept { return states_; }
    [[nodiscard]] std::vector<Dfa_state>& states() noexcept { return states_; }
    [[nodiscard]] const Dfa_state& state(State_id id) const { return states_[id]; }
    [[nodiscard]] Dfa_state& state(State_id id) { return states_[id]; }

    [[nodiscard]] std::size_t size() const noexcept { return states_.size(); }
    [[nodiscard]] State_id start_state() const noexcept { return start_state_; }
    void set_start_state(State_id s) noexcept { start_state_ = s; }

    [[nodiscard]] std::string to_dot() const;

private:
    std::vector<Dfa_state> states_{};
    State_id start_state_{0};
};

// Builds a deterministic finite automaton from an NFA via subset construction
class Dfa_builder {
public:
    explicit Dfa_builder(const Nfa& nfa) noexcept : nfa_{nfa} {}

    [[nodiscard]] Dfa build();

private:
    using State_set = std::vector<State_id>;

    [[nodiscard]] State_set epsilon_closure(const State_set& states) const;
    [[nodiscard]] Rule_id resolve_rule_id(const State_set& states) const noexcept;

    const Nfa& nfa_;
};

} // namespace lex

#endif // LEX_DFA_HPP
