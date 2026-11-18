#ifndef LEX_TEST_HARNESS_HPP
#define LEX_TEST_HARNESS_HPP

#include <iostream>
#include <string_view>

namespace lex::test {

inline int g_test_count = 0;
inline int g_test_failures = 0;

inline void run_test(std::string_view name, void (*test_func)()) {
    std::cout << "[ RUN      ] " << name << "\n";
    int initial_failures = g_test_failures;
    try {
        test_func();
        if (g_test_failures == initial_failures) {
            std::cout << "[       OK ] " << name << "\n";
        } else {
            std::cout << "[  FAILED  ] " << name << "\n";
        }
    } catch (const std::exception& e) {
        std::cerr << "[  FAILED  ] " << name << " threw exception: " << e.what() << "\n";
        ++g_test_failures;
    }
}

} // namespace lex::test

#define TEST_CHECK(cond) \
    do { \
        ++::lex::test::g_test_count; \
        if (!(cond)) { \
            std::cerr << "  CHECK FAILED: " << __FILE__ << ":" << __LINE__ << " (" #cond ")\n"; \
            ++::lex::test::g_test_failures; \
        } \
    } while (0)

#define TEST_CHECK_EQ(a, b) \
    do { \
        ++::lex::test::g_test_count; \
        if (!((a) == (b))) { \
            std::cerr << "  CHECK FAILED: " << __FILE__ << ":" << __LINE__ \
                      << " (" #a " == " #b ") [" << (a) << " != " << (b) << "]\n"; \
            ++::lex::test::g_test_failures; \
        } \
    } while (0)

#endif // LEX_TEST_HARNESS_HPP
