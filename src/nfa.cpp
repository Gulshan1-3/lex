#include "lex/nfa.hpp"
#include <sstream>

namespace lex {

Nfa_fragment Thompson_builder::build_char_class(const Char_set& cs) {
    State_id s = nfa_.add_state();
    State_id a = nfa_.add_state();
    nfa_.add_char_transition(s, cs, a);
    return {s, a};
}

Nfa_fragment Thompson_builder::build_concat(Nfa_fragment first, Nfa_fragment second) {
    nfa_.add_epsilon_transition(first.accept, second.start);
    return {first.start, second.accept};
}

Nfa_fragment Thompson_builder::build_union(Nfa_fragment first, Nfa_fragment second) {
    State_id s = nfa_.add_state();
    State_id a = nfa_.add_state();
    nfa_.add_epsilon_transition(s, first.start);
    nfa_.add_epsilon_transition(s, second.start);
    nfa_.add_epsilon_transition(first.accept, a);
    nfa_.add_epsilon_transition(second.accept, a);
    return {s, a};
}

Nfa_fragment Thompson_builder::build_star(Nfa_fragment child) {
    State_id s = nfa_.add_state();
    State_id a = nfa_.add_state();
    nfa_.add_epsilon_transition(s, child.start);
    nfa_.add_epsilon_transition(s, a);
    nfa_.add_epsilon_transition(child.accept, child.start);
    nfa_.add_epsilon_transition(child.accept, a);
    return {s, a};
}

Nfa_fragment Thompson_builder::build_plus(Nfa_fragment child) {
    State_id s = nfa_.add_state();
    State_id a = nfa_.add_state();
    nfa_.add_epsilon_transition(s, child.start);
    nfa_.add_epsilon_transition(child.accept, child.start);
    nfa_.add_epsilon_transition(child.accept, a);
    return {s, a};
}

Nfa_fragment Thompson_builder::build_question(Nfa_fragment child) {
    State_id s = nfa_.add_state();
    State_id a = nfa_.add_state();
    nfa_.add_epsilon_transition(s, child.start);
    nfa_.add_epsilon_transition(s, a);
    nfa_.add_epsilon_transition(child.accept, a);
    return {s, a};
}

Nfa_fragment Thompson_builder::build_repeat(const Regex_node& child_node, int min_m, int max_m) {
    if (min_m == 0 && max_m == -1) {
        return build_star(build(child_node));
    }
    if (min_m == 1 && max_m == -1) {
        return build_plus(build(child_node));
    }
    if (min_m == 0 && max_m == 1) {
        return build_question(build(child_node));
    }

    if (min_m == 0 && max_m == 0) {
        State_id s = nfa_.add_state();
        State_id a = nfa_.add_state();
        nfa_.add_epsilon_transition(s, a);
        return {s, a};
    }

    Nfa_fragment result{invalid_state, invalid_state};
    int count = 0;

    for (int i = 0; i < min_m; ++i) {
        auto frag = build(child_node);
        if (result.start == invalid_state) {
            result = frag;
        } else {
            result = build_concat(result, frag);
        }
        ++count;
    }

    if (max_m == -1) {
        auto star_part = build_star(build(child_node));
        if (result.start == invalid_state) {
            result = star_part;
        } else {
            result = build_concat(result, star_part);
        }
    } else {
        for (int i = count; i < max_m; ++i) {
            auto q_part = build_question(build(child_node));
            if (result.start == invalid_state) {
                result = q_part;
            } else {
                result = build_concat(result, q_part);
            }
        }
    }

    return result;
}

Nfa_fragment Thompson_builder::build(const Regex_node& node) {
    switch (node.kind()) {
        case Regex_kind::Char_class: {
            const auto& cc = static_cast<const Char_class_node&>(node);
            return build_char_class(cc.chars());
        }
        case Regex_kind::Concat: {
            const auto& concat = static_cast<const Concat_node&>(node);
            const auto& children = concat.children();
            if (children.empty()) {
                State_id s = nfa_.add_state();
                State_id a = nfa_.add_state();
                nfa_.add_epsilon_transition(s, a);
                return {s, a};
            }
            auto frag = build(*children.front());
            for (std::size_t i = 1; i < children.size(); ++i) {
                frag = build_concat(frag, build(*children[i]));
            }
            return frag;
        }
        case Regex_kind::Union: {
            const auto& un = static_cast<const Union_node&>(node);
            const auto& children = un.children();
            if (children.empty()) {
                State_id s = nfa_.add_state();
                State_id a = nfa_.add_state();
                return {s, a};
            }
            auto frag = build(*children.front());
            for (std::size_t i = 1; i < children.size(); ++i) {
                frag = build_union(frag, build(*children[i]));
            }
            return frag;
        }
        case Regex_kind::Repeat: {
            const auto& rep = static_cast<const Repeat_node&>(node);
            return build_repeat(*rep.child(), rep.min_matches(), rep.max_matches());
        }
    }
    return {invalid_state, invalid_state};
}

std::string Nfa::to_dot() const {
    std::ostringstream oss;
    oss << "digraph NFA {\n";
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

        for (State_id next : s.epsilon_transitions) {
            oss << "  " << s.id << " -> " << next << " [label=\"ε\"];\n";
        }

        for (const auto& [chars, next] : s.char_transitions) {
            oss << "  " << s.id << " -> " << next << " [label=\""
                << chars.to_string() << "\"];\n";
        }
    }

    oss << "}\n";
    return oss.str();
}

} // namespace lex
