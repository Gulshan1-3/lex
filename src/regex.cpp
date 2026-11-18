#include "lex/regex.hpp"
#include <cctype>
#include <stdexcept>

namespace lex {

std::unique_ptr<Regex_node> make_char_class(Char_set cs) {
    return std::make_unique<Char_class_node>(cs);
}

std::unique_ptr<Regex_node> make_literal(unsigned char c) {
    return std::make_unique<Char_class_node>(Char_set{c});
}

std::unique_ptr<Regex_node> make_concat(std::vector<std::unique_ptr<Regex_node>> children) {
    if (children.size() == 1) {
        return std::move(children.front());
    }
    return std::make_unique<Concat_node>(std::move(children));
}

std::unique_ptr<Regex_node> make_union(std::vector<std::unique_ptr<Regex_node>> children) {
    if (children.size() == 1) {
        return std::move(children.front());
    }
    return std::make_unique<Union_node>(std::move(children));
}

std::unique_ptr<Regex_node> make_star(std::unique_ptr<Regex_node> child) {
    return std::make_unique<Repeat_node>(std::move(child), 0, -1);
}

std::unique_ptr<Regex_node> make_plus(std::unique_ptr<Regex_node> child) {
    return std::make_unique<Repeat_node>(std::move(child), 1, -1);
}

std::unique_ptr<Regex_node> make_question(std::unique_ptr<Regex_node> child) {
    return std::make_unique<Repeat_node>(std::move(child), 0, 1);
}

std::unique_ptr<Regex_node> make_repeat(std::unique_ptr<Regex_node> child, int min_m, int max_m) {
    return std::make_unique<Repeat_node>(std::move(child), min_m, max_m);
}

bool Regex_parser::match(char expected) noexcept {
    if (!at_end() && pattern_[cursor_] == expected) {
        ++cursor_;
        return true;
    }
    return false;
}

std::unique_ptr<Regex_node> Regex_parser::parse() {
    cursor_ = 0;
    auto node = parse_union();
    if (!at_end()) {
        throw std::runtime_error("Unexpected character '" + std::string(1, peek()) + "' in regex");
    }
    return node;
}

std::unique_ptr<Regex_node> Regex_parser::parse_union() {
    std::vector<std::unique_ptr<Regex_node>> branches;
    branches.push_back(parse_concat());

    while (match('|')) {
        branches.push_back(parse_concat());
    }

    return make_union(std::move(branches));
}

std::unique_ptr<Regex_node> Regex_parser::parse_concat() {
    std::vector<std::unique_ptr<Regex_node>> factors;

    while (!at_end() && peek() != '|' && peek() != ')') {
        factors.push_back(parse_repeat());
    }

    if (factors.empty()) {
        // Epsilon: empty concatenation
        return std::make_unique<Concat_node>(std::vector<std::unique_ptr<Regex_node>>{});
    }

    return make_concat(std::move(factors));
}

std::unique_ptr<Regex_node> Regex_parser::parse_repeat() {
    auto child = parse_primary();

    while (!at_end()) {
        if (match('*')) {
            child = make_star(std::move(child));
        } else if (match('+')) {
            child = make_plus(std::move(child));
        } else if (match('?')) {
            child = make_question(std::move(child));
        } else if (peek() == '{' && cursor_ + 1 < pattern_.size() &&
                   std::isdigit(static_cast<unsigned char>(pattern_[cursor_ + 1]))) {
            // Counted repetition: {m} or {m,n} or {m,}
            advance(); // consume '{'
            int min_m = 0;
            while (!at_end() && std::isdigit(static_cast<unsigned char>(peek()))) {
                min_m = min_m * 10 + (advance() - '0');
            }
            int max_m = min_m;
            if (match(',')) {
                if (match('}')) {
                    max_m = -1; // {m,} means unbounded
                } else {
                    max_m = 0;
                    while (!at_end() && std::isdigit(static_cast<unsigned char>(peek()))) {
                        max_m = max_m * 10 + (advance() - '0');
                    }
                    if (!match('}')) {
                        throw std::runtime_error("Expected '}' in counted repeat quantifier");
                    }
                }
            } else {
                if (!match('}')) {
                    throw std::runtime_error("Expected '}' in counted repeat quantifier");
                }
            }
            child = make_repeat(std::move(child), min_m, max_m);
        } else {
            break;
        }
    }

    return child;
}

std::unique_ptr<Regex_node> Regex_parser::parse_primary() {
    if (at_end()) {
        throw std::runtime_error("Unexpected end of pattern");
    }

    char c = peek();
    if (c == '(') {
        advance();
        auto inner = parse_union();
        if (!match(')')) {
            throw std::runtime_error("Mismatched parenthesis: missing ')'");
        }
        return inner;
    }

    if (c == '[') {
        return parse_bracket_class();
    }

    if (c == '"') {
        return parse_quoted_string();
    }

    if (c == '{') {
        return parse_macro_or_repeat();
    }

    if (c == '.') {
        advance();
        return make_char_class(Char_set::any_except_newline());
    }

    if (c == '\\') {
        advance();
        unsigned char esc = parse_escape_sequence();
        return make_literal(esc);
    }

    // Reserved regex meta-characters that shouldn't appear naked as primary
    if (c == ')' || c == '|' || c == '*' || c == '+' || c == '?') {
        throw std::runtime_error("Unexpected regex metacharacter '" + std::string(1, c) + "'");
    }

    advance();
    return make_literal(static_cast<unsigned char>(c));
}

std::unique_ptr<Regex_node> Regex_parser::parse_quoted_string() {
    advance(); // consume opening '"'
    std::vector<std::unique_ptr<Regex_node>> literals;

    while (!at_end() && peek() != '"') {
        if (peek() == '\\') {
            advance();
            literals.push_back(make_literal(parse_escape_sequence()));
        } else {
            literals.push_back(make_literal(static_cast<unsigned char>(advance())));
        }
    }

    if (!match('"')) {
        throw std::runtime_error("Unclosed quoted string in pattern");
    }

    return make_concat(std::move(literals));
}

std::unique_ptr<Regex_node> Regex_parser::parse_bracket_class() {
    advance(); // consume '['
    bool negated = match('^');
    Char_set cs;

    // Handle leading ']' or '-' as literal member of the class
    if (!at_end() && (peek() == ']' || peek() == '-')) {
        cs.set(static_cast<unsigned char>(advance()));
    }

    while (!at_end() && peek() != ']') {
        unsigned char start_char = 0;
        if (peek() == '\\') {
            advance();
            start_char = parse_escape_sequence();
        } else {
            start_char = static_cast<unsigned char>(advance());
        }

        if (peek() == '-' && cursor_ + 1 < pattern_.size() && pattern_[cursor_ + 1] != ']') {
            advance(); // consume '-'
            unsigned char end_char = 0;
            if (peek() == '\\') {
                advance();
                end_char = parse_escape_sequence();
            } else {
                end_char = static_cast<unsigned char>(advance());
            }

            if (start_char > end_char) {
                std::swap(start_char, end_char);
            }
            cs.set_range(start_char, end_char);
        } else {
            cs.set(start_char);
        }
    }

    if (!match(']')) {
        throw std::runtime_error("Unclosed character class '['");
    }

    if (negated) {
        cs.invert();
    }

    return make_char_class(cs);
}

std::unique_ptr<Regex_node> Regex_parser::parse_macro_or_repeat() {
    advance(); // consume '{'
    std::string name;
    while (!at_end() && peek() != '}') {
        name += advance();
    }

    if (!match('}')) {
        throw std::runtime_error("Unclosed macro reference '{'");
    }

    if (macros_ == nullptr) {
        throw std::runtime_error("Undefined macro '{" + name + "}' (no macro table provided)");
    }

    auto it = macros_->find(name);
    if (it == macros_->end()) {
        throw std::runtime_error("Undefined macro '{" + name + "}'");
    }

    // Recursively parse expanded macro
    Regex_parser sub_parser(it->second, *macros_);
    return sub_parser.parse();
}

unsigned char Regex_parser::parse_escape_sequence() {
    if (at_end()) {
        return '\\';
    }

    char c = advance();
    switch (c) {
        case 'n': return '\n';
        case 't': return '\t';
        case 'r': return '\r';
        case 'f': return '\f';
        case 'v': return '\v';
        case 'a': return '\a';
        case 'b': return '\b';
        case '0': return '\0';
        case '\\': return '\\';
        case '\'': return '\'';
        case '\"': return '\"';
        case 'x': {
            // Hexadecimal \xHH
            unsigned char val = 0;
            for (int i = 0; i < 2 && !at_end() && std::isxdigit(static_cast<unsigned char>(peek())); ++i) {
                char h = advance();
                val = static_cast<unsigned char>(
                    val * 16 + (std::isdigit(static_cast<unsigned char>(h))
                                    ? (h - '0')
                                    : (std::tolower(static_cast<unsigned char>(h)) - 'a' + 10)));
            }
            return val;
        }
        default:
            return static_cast<unsigned char>(c);
    }
}

} // namespace lex
