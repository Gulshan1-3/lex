#ifndef LEX_CHAR_SET_HPP
#define LEX_CHAR_SET_HPP

#include <bitset>
#include <cstddef>
#include <string>
#include <string_view>

namespace lex {

// Concrete class representing a set of 8-bit characters (ASCII 0..255).
// Implements value semantics with bitset-level efficiency.
class Char_set {
public:
    static constexpr std::size_t alphabet_size = 256;

    constexpr Char_set() noexcept = default;

    explicit Char_set(unsigned char c) noexcept {
        bits_.set(c);
    }

    Char_set(unsigned char low, unsigned char high) noexcept {
        set_range(low, high);
    }

    void set(unsigned char c, bool val = true) noexcept {
        bits_.set(c, val);
    }

    void reset(unsigned char c) noexcept {
        bits_.reset(c);
    }

    void set_range(unsigned char low, unsigned char high, bool val = true) noexcept {
        for (std::size_t i = low; i <= high; ++i) {
            bits_.set(i, val);
        }
    }

    [[nodiscard]] bool test(unsigned char c) const noexcept {
        return bits_.test(c);
    }

    [[nodiscard]] bool operator[](unsigned char c) const noexcept {
        return bits_.test(c);
    }

    [[nodiscard]] bool empty() const noexcept {
        return bits_.none();
    }

    [[nodiscard]] bool any() const noexcept {
        return bits_.any();
    }

    [[nodiscard]] bool all() const noexcept {
        return bits_.all();
    }

    [[nodiscard]] std::size_t count() const noexcept {
        return bits_.count();
    }

    void invert() noexcept {
        bits_.flip();
    }

    Char_set& operator|=(const Char_set& other) noexcept {
        bits_ |= other.bits_;
        return *this;
    }

    Char_set& operator&=(const Char_set& other) noexcept {
        bits_ &= other.bits_;
        return *this;
    }

    Char_set& operator-=(const Char_set& other) noexcept {
        bits_ &= ~other.bits_;
        return *this;
    }

    [[nodiscard]] friend Char_set operator|(Char_set lhs, const Char_set& rhs) noexcept {
        lhs |= rhs;
        return lhs;
    }

    [[nodiscard]] friend Char_set operator&(Char_set lhs, const Char_set& rhs) noexcept {
        lhs &= rhs;
        return lhs;
    }

    [[nodiscard]] friend Char_set operator-(Char_set lhs, const Char_set& rhs) noexcept {
        lhs -= rhs;
        return lhs;
    }

    [[nodiscard]] friend Char_set operator~(Char_set cs) noexcept {
        cs.invert();
        return cs;
    }

    [[nodiscard]] friend bool operator==(const Char_set& lhs, const Char_set& rhs) noexcept {
        return lhs.bits_ == rhs.bits_;
    }

    [[nodiscard]] friend bool operator!=(const Char_set& lhs, const Char_set& rhs) noexcept {
        return !(lhs == rhs);
    }

    // Factory functions
    [[nodiscard]] static Char_set all_chars() noexcept {
        Char_set cs;
        cs.bits_.set();
        return cs;
    }

    [[nodiscard]] static Char_set none_chars() noexcept {
        return Char_set{};
    }

    [[nodiscard]] static Char_set any_except_newline() noexcept {
        Char_set cs = all_chars();
        cs.reset('\n');
        return cs;
    }

    [[nodiscard]] static Char_set from_range(unsigned char low, unsigned char high) noexcept {
        return Char_set{low, high};
    }

    // Diagnostic string description (e.g. "[a-z0-9]")
    [[nodiscard]] std::string to_string() const;

private:
    std::bitset<alphabet_size> bits_{};
};

} // namespace lex

#endif // LEX_CHAR_SET_HPP
