#include "lex/spec_parser.hpp"
#include <cctype>
#include <sstream>
#include <stdexcept>

namespace lex {

namespace {

std::string trim(std::string_view sv) {
    while (!sv.empty() && std::isspace(static_cast<unsigned char>(sv.front()))) {
        sv.remove_prefix(1);
    }
    while (!sv.empty() && std::isspace(static_cast<unsigned char>(sv.back()))) {
        sv.remove_suffix(1);
    }
    return std::string{sv};
}

} // namespace

Spec_file Spec_parser::parse() {
    Spec_file spec;
    cursor_ = 0;
    current_line_ = 1;

    parse_definitions_section(spec);
    parse_rules_section(spec);
    parse_epilogue_section(spec);

    // Resolve fall-through rules (action == "|")
    for (std::size_t i = spec.rules.size(); i > 0; --i) {
        std::size_t idx = i - 1;
        if (trim(spec.rules[idx].action) == "|") {
            if (idx + 1 < spec.rules.size()) {
                spec.rules[idx].action = spec.rules[idx + 1].action;
            } else {
                throw std::runtime_error("Trailing '|' rule has no subsequent rule to inherit action from");
            }
        }
    }

    return spec;
}

std::string Spec_parser::read_until_newline() {
    std::string line;
    while (!at_end() && peek() != '\n') {
        line += advance();
    }
    if (!at_end() && peek() == '\n') {
        advance();
        ++current_line_;
    }
    return line;
}

void Spec_parser::parse_definitions_section(Spec_file& spec) {
    while (!at_end()) {
        std::string raw_line = read_until_newline();
        std::string line = trim(raw_line);

        if (line.rfind("%%", 0) == 0) {
            // End of Section 1
            return;
        }

        if (line.rfind("%{", 0) == 0) {
            // Prologue block
            std::string prologue_block;
            while (!at_end()) {
                std::string p_line = read_until_newline();
                std::string trimmed_p = trim(p_line);
                if (trimmed_p.rfind("%}", 0) == 0) {
                    break;
                }
                prologue_block += p_line + "\n";
            }
            spec.prologue += prologue_block;
            continue;
        }

        if (line.empty() || line[0] == '#' || line.rfind("//", 0) == 0) {
            continue;
        }

        // Definition: NAME REGEX
        std::size_t first_space = 0;
        while (first_space < line.size() && !std::isspace(static_cast<unsigned char>(line[first_space]))) {
            ++first_space;
        }

        if (first_space < line.size()) {
            std::string name = line.substr(0, first_space);
            std::string regex = trim(line.substr(first_space));
            spec.definitions[name] = regex;
        }
    }
}

void Spec_parser::parse_rules_section(Spec_file& spec) {
    while (!at_end()) {
        // Skip whitespace and empty lines, tracking line numbers
        while (!at_end()) {
            char c = peek();
            if (c == '\n') {
                advance();
                ++current_line_;
            } else if (std::isspace(static_cast<unsigned char>(c))) {
                advance();
            } else {
                break;
            }
        }

        if (at_end()) {
            break;
        }

        // Check for section separator "%%"
        if (peek() == '%' && cursor_ + 1 < content_.size() && content_[cursor_ + 1] == '%') {
            advance();
            advance();
            // Consume rest of line after %%
            read_until_newline();
            return;
        }

        // Rule pattern starts here
        int rule_line = current_line_;
        std::string pattern;

        // Parse regex until unescaped, unquoted whitespace
        bool in_quotes = false;
        bool in_bracket = false;

        while (!at_end()) {
            char c = peek();
            if (c == '\n') {
                break;
            }

            if (!in_quotes && !in_bracket && std::isspace(static_cast<unsigned char>(c))) {
                break;
            }

            if (c == '\\') {
                pattern += advance();
                if (!at_end()) {
                    pattern += advance();
                }
                continue;
            }

            if (c == '"' && !in_bracket) {
                in_quotes = !in_quotes;
            } else if (c == '[' && !in_quotes) {
                in_bracket = true;
            } else if (c == ']' && in_bracket) {
                in_bracket = false;
            }

            pattern += advance();
        }

        // Skip whitespace between pattern and action
        while (!at_end() && peek() != '\n' && std::isspace(static_cast<unsigned char>(peek()))) {
            advance();
        }

        // Read action
        std::string action;
        if (!at_end() && peek() == '|') {
            advance();
            action = "|";
            read_until_newline();
        } else if (!at_end() && peek() == '{') {
            int depth = 0;
            bool action_in_quotes = false;
            bool action_in_char = false;

            while (!at_end()) {
                char c = advance();
                action += c;

                if (c == '\n') {
                    ++current_line_;
                }

                if (c == '\\' && !at_end() && (action_in_quotes || action_in_char)) {
                    action += advance();
                    continue;
                }

                if (c == '"' && !action_in_char) {
                    action_in_quotes = !action_in_quotes;
                } else if (c == '\'' && !action_in_quotes) {
                    action_in_char = !action_in_char;
                } else if (!action_in_quotes && !action_in_char) {
                    if (c == '{') {
                        ++depth;
                    } else if (c == '}') {
                        --depth;
                        if (depth == 0) {
                            break;
                        }
                    }
                }
            }
        } else {
            // Action until end of line
            action = read_until_newline();
        }

        Rule_id id = spec.rules.size();
        spec.rules.push_back(Rule{
            .id = id,
            .pattern = pattern,
            .action = action,
            .line_number = rule_line
        });
    }
}

void Spec_parser::parse_epilogue_section(Spec_file& spec) {
    if (at_end()) {
        return;
    }
    spec.epilogue = std::string(content_.substr(cursor_));
    cursor_ = content_.size();
}

} // namespace lex
