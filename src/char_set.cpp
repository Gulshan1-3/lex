#include "lex/char_set.hpp"
#include <iomanip>
#include <sstream>

namespace lex {

namespace {

std::string format_char(unsigned char c) {
    switch (c) {
        case '\n': return "\\n";
        case '\t': return "\\t";
        case '\r': return "\\r";
        case '\\': return "\\\\";
        case '\'': return "\\'";
        case '\"': return "\\\"";
        case '\0': return "\\0";
        default:
            if (c >= 32 && c <= 126) {
                return std::string(1, static_cast<char>(c));
            }
            std::ostringstream oss;
            oss << "\\x" << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(c);
            return oss.str();
    }
}

} // namespace

std::string Char_set::to_string() const {
    if (all()) {
        return "[ANY]";
    }
    if (empty()) {
        return "[EMPTY]";
    }
    if (*this == any_except_newline()) {
        return ".";
    }

    std::string result = "[";
    std::size_t i = 0;
    while (i < alphabet_size) {
        if (!test(static_cast<unsigned char>(i))) {
            ++i;
            continue;
        }

        std::size_t start = i;
        while (i + 1 < alphabet_size && test(static_cast<unsigned char>(i + 1))) {
            ++i;
        }
        std::size_t end = i;

        if (end == start) {
            result += format_char(static_cast<unsigned char>(start));
        } else if (end == start + 1) {
            result += format_char(static_cast<unsigned char>(start));
            result += format_char(static_cast<unsigned char>(end));
        } else {
            result += format_char(static_cast<unsigned char>(start));
            result += "-";
            result += format_char(static_cast<unsigned char>(end));
        }
        ++i;
    }
    result += "]";
    return result;
}

} // namespace lex
