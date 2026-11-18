#ifndef LEX_SPEC_PARSER_HPP
#define LEX_SPEC_PARSER_HPP

#include "lex/nfa.hpp"
#include <map>
#include <string>
#include <string_view>
#include <vector>

namespace lex {

struct Rule {
    Rule_id id{no_rule};
    std::string pattern{};
    std::string action{};
    int line_number{0};
};

struct Spec_file {
    std::string prologue{};
    std::map<std::string, std::string> definitions{};
    std::vector<Rule> rules{};
    std::string epilogue{};
};

class Spec_parser {
public:
    explicit Spec_parser(std::string_view content) : content_{content} {}

    [[nodiscard]] Spec_file parse();

private:
    [[nodiscard]] bool at_end() const noexcept { return cursor_ >= content_.size(); }
    [[nodiscard]] char peek() const noexcept { return at_end() ? '\0' : content_[cursor_]; }
    char advance() noexcept { return at_end() ? '\0' : content_[cursor_++]; }

    void skip_whitespace_and_comments();
    std::string read_until_newline();

    void parse_definitions_section(Spec_file& spec);
    void parse_rules_section(Spec_file& spec);
    void parse_epilogue_section(Spec_file& spec);

    [[nodiscard]] std::pair<std::string, std::string> parse_rule_line(int line_num);

    std::string_view content_;
    std::size_t cursor_{0};
    int current_line_{1};
};

} // namespace lex

#endif // LEX_SPEC_PARSER_HPP
