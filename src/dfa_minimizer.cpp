#include "lex/dfa_minimizer.hpp"
#include <algorithm>
#include <map>
#include <queue>

namespace lex {

namespace {

constexpr std::size_t invalid_group = std::numeric_limits<std::size_t>::max();

} // namespace

Dfa Dfa_minimizer::minimize() {
    if (dfa_.size() == 0) {
        return Dfa{};
    }

    // Step 1: Reachability analysis - discard unreachable states
    std::vector<bool> reachable(dfa_.size(), false);
    std::queue<State_id> q;
    q.push(dfa_.start_state());
    reachable[dfa_.start_state()] = true;

    while (!q.empty()) {
        State_id curr = q.front();
        q.pop();

        for (std::size_t c = 0; c < Char_set::alphabet_size; ++c) {
            State_id nxt = dfa_.transition(curr, static_cast<unsigned char>(c));
            if (nxt != invalid_state && !reachable[nxt]) {
                reachable[nxt] = true;
                q.push(nxt);
            }
        }
    }

    // Step 2: Initial partitioning based on acceptance and rule ID
    std::map<Rule_id, std::vector<State_id>> initial_groups;
    for (State_id s = 0; s < dfa_.size(); ++s) {
        if (!reachable[s]) {
            continue;
        }
        initial_groups[dfa_.state(s).rule_id].push_back(s);
    }

    std::vector<std::vector<State_id>> partitions;
    partitions.reserve(initial_groups.size());
    for (auto& [rule, states] : initial_groups) {
        partitions.push_back(std::move(states));
    }

    std::vector<std::size_t> state_to_group(dfa_.size(), invalid_group);
    auto rebuild_state_to_group = [&]() {
        for (std::size_t g = 0; g < partitions.size(); ++g) {
            for (State_id s : partitions[g]) {
                state_to_group[s] = g;
            }
        }
    };
    rebuild_state_to_group();

    auto target_group = [&](State_id s, unsigned char c) -> std::size_t {
        State_id nxt = dfa_.transition(s, c);
        if (nxt == invalid_state || !reachable[nxt]) {
            return invalid_group;
        }
        return state_to_group[nxt];
    };

    // Step 3: Partition refinement
    bool changed = true;
    while (changed) {
        changed = false;
        std::vector<std::vector<State_id>> next_partitions;

        for (const auto& group : partitions) {
            if (group.size() <= 1) {
                next_partitions.push_back(group);
                continue;
            }

            // Subdivide group based on transition signatures
            std::vector<std::vector<State_id>> sub_groups;
            for (State_id state : group) {
                bool placed = false;
                for (auto& sub : sub_groups) {
                    State_id rep = sub.front();
                    bool equivalent = true;
                    for (std::size_t c = 0; c < Char_set::alphabet_size; ++c) {
                        unsigned char ch = static_cast<unsigned char>(c);
                        if (target_group(state, ch) != target_group(rep, ch)) {
                            equivalent = false;
                            break;
                        }
                    }
                    if (equivalent) {
                        sub.push_back(state);
                        placed = true;
                        break;
                    }
                }
                if (!placed) {
                    sub_groups.push_back({state});
                }
            }

            if (sub_groups.size() > 1) {
                changed = true;
            }

            for (auto& sub : sub_groups) {
                next_partitions.push_back(std::move(sub));
            }
        }

        partitions = std::move(next_partitions);
        rebuild_state_to_group();
    }

    // Step 4: Construct minimized DFA
    // Ensure the group containing the original start state becomes state 0
    std::size_t start_group_idx = state_to_group[dfa_.start_state()];
    if (start_group_idx != 0) {
        std::swap(partitions[0], partitions[start_group_idx]);
        rebuild_state_to_group();
    }

    Dfa min_dfa;
    std::vector<State_id> group_to_new_state(partitions.size());

    for (std::size_t g = 0; g < partitions.size(); ++g) {
        State_id rep = partitions[g].front();
        State_id new_id = min_dfa.add_state(dfa_.state(rep).rule_id);
        group_to_new_state[g] = new_id;
    }

    min_dfa.set_start_state(0);

    for (std::size_t g = 0; g < partitions.size(); ++g) {
        State_id rep = partitions[g].front();
        State_id from_new = group_to_new_state[g];

        for (std::size_t c = 0; c < Char_set::alphabet_size; ++c) {
            unsigned char ch = static_cast<unsigned char>(c);
            State_id nxt = dfa_.transition(rep, ch);
            if (nxt != invalid_state && reachable[nxt]) {
                std::size_t target_g = state_to_group[nxt];
                State_id to_new = group_to_new_state[target_g];
                min_dfa.set_transition(from_new, ch, to_new);
            }
        }
    }

    return min_dfa;
}

} // namespace lex
