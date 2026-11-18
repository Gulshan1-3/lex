#ifndef LEX_REGEX_HPP
#define LEX_REGEX_HPP

#include "lex/char_set.hpp"
#include <map>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace lex {

enum class Regex_kind {
    Char_class,
    Concat,
    Union,
    Repeat
};

class Regex_node {
public:
    explicit Regex_node(Regex_kind kind) noexcept : kind_{kind} {}
    virtual ~Regex_node() = default;

    Regex_node(const Regex_node&) = delete;
    Regex_node& operator=(const Regex_node&) = delete;
    Regex_node(Regex_node&&) noexcept = default;
    Regex_node& operator=(Regex_node&&) noexcept = default;

    [[nodiscard]] Regex_kind kind() const noexcept { return kind_; }

private:
    Regex_kind kind_;
};

class Char_class_node final : public Regex_node {
public:
    explicit Char_class_node(Char_set chars) noexcept
        : Regex_node{Regex_kind::Char_class}, chars_{chars} {}

    [[nodiscard]] const Char_set& chars() const noexcept { return chars_; }
    [[nodiscard]] Char_set& chars() noexcept { return chars_; }

private:
    Char_set chars_;
};

class Concat_node final : public Regex_node {
public:
    explicit Concat_node(std::vector<std::unique_ptr<Regex_node>> children) noexcept
        : Regex_node{Regex_kind::Concat}, children_{std::move(children)} {}

    [[nodiscard]] const std::vector<std::unique_ptr<Regex_node>>& children() const noexcept {
        return children_;
    }

private:
    std::vector<std::unique_ptr<Regex_node>> children_;
};

class Union_node final : public Regex_node {
public:
    explicit Union_node(std::vector<std::unique_ptr<Regex_node>> children) noexcept
        : Regex_node{Regex_kind::Union}, children_{std::move(children)} {}

    [[nodiscard]] const std::vector<std::unique_ptr<Regex_node>>& children() const noexcept {
        return children_;
    }

private:
    std::vector<std::unique_ptr<Regex_node>> children_;
};

class Repeat_node final : public Regex_node {
public:
    Repeat_node(std::unique_ptr<Regex_node> child, int min_matches, int max_matches) noexcept
        : Regex_node{Regex_kind::Repeat},
          child_{std::move(child)},
          min_matches_{min_matches},
          max_matches_{max_matches} {}

    [[nodiscard]] const Regex_node* child() const noexcept { return child_.get(); }
    [[nodiscard]] int min_matches() const noexcept { return min_matches_; }
    [[nodiscard]] int max_matches() const noexcept { return max_matches_; }

private:
    std::unique_ptr<Regex_node> child_;
    int min_matches_{0};
    int max_matches_{-1}; // -1 means unbounded (infinity)
};

// Factory functions for clean expression composition
[[nodiscard]] std::unique_ptr<Regex_node> make_char_class(Char_set cs);
[[nodiscard]] std::unique_ptr<Regex_node> make_literal(unsigned char c);
[[nodiscard]] std::unique_ptr<Regex_node> make_concat(std::vector<std::unique_ptr<Regex_node>> children);
[[nodiscard]] std::unique_ptr<Regex_node> make_union(std::vector<std::unique_ptr<Regex_node>> children);
[[nodiscard]] std::unique_ptr<Regex_node> make_star(std::unique_ptr<Regex_node> child);
[[nodiscard]] std::unique_ptr<Regex_node> make_plus(std::unique_ptr<Regex_node> child);
[[nodiscard]] std::unique_ptr<Regex_node> make_question(std::unique_ptr<Regex_node> child);
[[nodiscard]] std::unique_ptr<Regex_node> make_repeat(std::unique_ptr<Regex_node> child, int min_m, int max_m);

// Recursive-descent parser for Lex regular expressions
class Regex_parser {
public:
    using Macro_table = std::map<std::string, std::string>;

    explicit Regex_parser(std::string_view pattern, const Macro_table& macros = {})
        : pattern_{pattern}, macros_{&macros} {}

    [[nodiscard]] std::unique_ptr<Regex_node> parse();

private:
    [[nodiscard]] bool at_end() const noexcept { return cursor_ >= pattern_.size(); }
    [[nodiscard]] char peek() const noexcept { return at_end() ? '\0' : pattern_[cursor_]; }
    char advance() noexcept { return at_end() ? '\0' : pattern_[cursor_++]; }
    [[nodiscard]] bool match(char expected) noexcept;

    [[nodiscard]] std::unique_ptr<Regex_node> parse_union();
    [[nodiscard]] std::unique_ptr<Regex_node> parse_concat();
    [[nodiscard]] std::unique_ptr<Regex_node> parse_repeat();
    [[nodiscard]] std::unique_ptr<Regex_node> parse_primary();

    [[nodiscard]] std::unique_ptr<Regex_node> parse_quoted_string();
    [[nodiscard]] std::unique_ptr<Regex_node> parse_bracket_class();
    [[nodiscard]] std::unique_ptr<Regex_node> parse_macro_or_repeat();

    [[nodiscard]] unsigned char parse_escape_sequence();

    std::string_view pattern_;
    const Macro_table* macros_{nullptr};
    std::size_t cursor_{0};
};

} // namespace lex

#endif // LEX_REGEX_HPP
