#include "lex/dfa.hpp"
#include <algorithm>
#include <map>
#include <sstream>

namespace lex {

Dfa_builder::State_set Dfa_builder::epsilon_closure(const State_set& initial_states) const {
    if (initial_states.empty()) {
        return {};
    }

    std::vector<bool> in_closure(nfa_.size(), false);
    std::vector<State_id> stack;
    stack.reserve(initial_states.size());

    for (State_id s : initial_states) {
        if (!in_closure[s]) {
            in_closure[s] = true;
            stack.push_back(s);
        }
    }

    while (!stack.empty()) {
        State_id curr = stack.back();
        stack.pop_back();

        for (State_id next : nfa_.state(curr).epsilon_transitions) {
            if (!in_closure[next]) {
                in_closure[next] = true;
                stack.push_back(next);
            }
        }
    }

    State_set result;
    result.reserve(in_closure.size());
    for (std::size_t i = 0; i < in_closure.size(); ++i) {
        if (in_closure[i]) {
            result.push_back(i);
        }
    }
    return result;
}

Rule_id Dfa_builder::resolve_rule_id(const State_set& states) const noexcept {
    Rule_id best = no_rule;
    for (State_id s : states) {
        Rule_id r = nfa_.state(s).rule_id;
        if (r != no_rule && (best == no_rule || r < best)) {
            best = r;
        }
    }
    return best;
}

Dfa Dfa_builder::build() {
    Dfa dfa;
    if (nfa_.size() == 0) {
        return dfa;
    }

    std::map<State_set, State_id> subset_map;
    std::vector<State_set> dfa_subsets;

    State_set start_closure = epsilon_closure({nfa_.start_state()});
    Rule_id start_rule = resolve_rule_id(start_closure);
    State_id dfa_start = dfa.add_state(start_rule);
    dfa.set_start_state(dfa_start);

    subset_map[start_closure] = dfa_start;
    dfa_subsets.push_back(start_closure);

    std::size_t processed = 0;
    while (processed < dfa_subsets.size()) {
        State_id curr_dfa_id = processed;
        const State_set curr_subset = dfa_subsets[processed++];

        for (std::size_t c = 0; c < Char_set::alphabet_size; ++c) {
            unsigned char ch = static_cast<unsigned char>(c);
            State_set move_set;

            for (State_id s : curr_subset) {
                for (const auto& [chars, target] : nfa_.state(s).char_transitions) {
                    if (chars.test(ch)) {
                        move_set.push_back(target);
                    }
                }
            }

            if (move_set.empty()) {
                continue;
            }

            State_set target_closure = epsilon_closure(move_set);
            if (target_closure.empty()) {
                continue;
            }

            State_id target_dfa_id = invalid_state;
            auto it = subset_map.find(target_closure);
            if (it != subset_map.end()) {
                target_dfa_id = it->second;
            } else {
                Rule_id rule = resolve_rule_id(target_closure);
                target_dfa_id = dfa.add_state(rule);
                subset_map[target_closure] = target_dfa_id;
                dfa_subsets.push_back(std::move(target_closure));
            }

            dfa.set_transition(curr_dfa_id, ch, target_dfa_id);
        }
    }

    return dfa;
}

std::string Dfa::to_dot() const {
    std::ostringstream oss;
    oss << "digraph DFA {\n";
    oss << "  rankdir=LR;\n";
    oss << "  node [shape=circle];\n";
    oss << "  start [shape=point];\n";
    oss << "  start -> " << start_state_ << ";\n";

    for (const auto& s : states_) {
        if (s.is_accepting()) {
            oss << "  " << s.id << " [shape=doublecircle, label=\"" << s.id
                << "\\n(rule " << s.rule_id << ")\"];\n";
        } else {
            oss << "  " << s.id << " [shape=circle];\n";
        }

        // Aggregate transitions between same (from, to) into Char_set for compact display
        std::map<State_id, Char_set> grouped_edges;
        for (std::size_t c = 0; c < Char_set::alphabet_size; ++c) {
            State_id target = s.transitions[c];
            if (target != invalid_state) {
                grouped_edges[target].set(static_cast<unsigned char>(c));
            }
        }

        for (const auto& [target, char_set] : grouped_edges) {
            oss << "  " << s.id << " -> " << target << " [label=\""
                << char_set.to_string() << "\"];\n";
        }
    }

    oss << "}\n";
    return oss.str();
}

} // namespace lex
