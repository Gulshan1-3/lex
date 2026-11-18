#ifndef LEX_NFA_HPP
#define LEX_NFA_HPP

#include "lex/char_set.hpp"
#include "lex/regex.hpp"
#include <cstddef>
#include <limits>
#include <string>
#include <utility>
#include <vector>

namespace lex {

using State_id = std::size_t;
using Rule_id = std::size_t;

inline constexpr State_id invalid_state = std::numeric_limits<State_id>::max();
inline constexpr Rule_id no_rule = std::numeric_limits<Rule_id>::max();

struct Nfa_state {
    State_id id{invalid_state};
    Rule_id rule_id{no_rule};
    std::vector<State_id> epsilon_transitions{};
    std::vector<std::pair<Char_set, State_id>> char_transitions{};

    [[nodiscard]] bool is_accepting() const noexcept { return rule_id != no_rule; }
};

struct Nfa_fragment {
    State_id start{invalid_state};
    State_id accept{invalid_state};
};

class Nfa {
public:
    Nfa() = default;

    State_id add_state() {
        State_id id = states_.size();
        states_.push_back(Nfa_state{.id = id});
        return id;
    }

    void add_epsilon_transition(State_id from, State_id to) {
        states_[from].epsilon_transitions.push_back(to);
    }

    void add_char_transition(State_id from, const Char_set& chars, State_id to) {
        states_[from].char_transitions.emplace_back(chars, to);
    }

    void set_rule(State_id state, Rule_id rule) noexcept {
        states_[state].rule_id = rule;
    }

    [[nodiscard]] const std::vector<Nfa_state>& states() const noexcept { return states_; }
    [[nodiscard]] std::vector<Nfa_state>& states() noexcept { return states_; }
    [[nodiscard]] const Nfa_state& state(State_id id) const { return states_[id]; }
    [[nodiscard]] Nfa_state& state(State_id id) { return states_[id]; }

    [[nodiscard]] std::size_t size() const noexcept { return states_.size(); }
    [[nodiscard]] State_id start_state() const noexcept { return start_state_; }
    void set_start_state(State_id s) noexcept { start_state_ = s; }

    [[nodiscard]] std::string to_dot() const;

private:
    std::vector<Nfa_state> states_{};
    State_id start_state_{0};
};

// Builder applying Thompson's construction to compile Regex AST into NFA
class Thompson_builder {
public:
    explicit Thompson_builder(Nfa& target_nfa) noexcept : nfa_{target_nfa} {}

    [[nodiscard]] Nfa_fragment build(const Regex_node& node);

    [[nodiscard]] Nfa_fragment build_char_class(const Char_set& cs);
    [[nodiscard]] Nfa_fragment build_concat(Nfa_fragment first, Nfa_fragment second);
    [[nodiscard]] Nfa_fragment build_union(Nfa_fragment first, Nfa_fragment second);
    [[nodiscard]] Nfa_fragment build_star(Nfa_fragment child);
    [[nodiscard]] Nfa_fragment build_plus(Nfa_fragment child);
    [[nodiscard]] Nfa_fragment build_question(Nfa_fragment child);
    [[nodiscard]] Nfa_fragment build_repeat(const Regex_node& child_node, int min_m, int max_m);

private:
    Nfa& nfa_;
};

} // namespace lex

#endif // LEX_NFA_HPP
