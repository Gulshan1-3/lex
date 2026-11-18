#include "lex/char_set.hpp"
#include "test_harness.hpp"

void test_char_set_basics() {
    lex::Char_set cs;
    TEST_CHECK(cs.empty());
    TEST_CHECK_EQ(cs.count(), 0);

    cs.set('a');
    TEST_CHECK(!cs.empty());
    TEST_CHECK(cs.test('a'));
    TEST_CHECK(!cs.test('b'));
    TEST_CHECK_EQ(cs.count(), 1);

    cs.set_range('0', '9');
    TEST_CHECK_EQ(cs.count(), 11);
    TEST_CHECK(cs.test('0'));
    TEST_CHECK(cs.test('5'));
    TEST_CHECK(cs.test('9'));
    TEST_CHECK(!cs.test('/'));
    TEST_CHECK(!cs.test(':'));

    cs.reset('5');
    TEST_CHECK(!cs.test('5'));
    TEST_CHECK_EQ(cs.count(), 10);
}

void test_char_set_operations() {
    auto digits = lex::Char_set::from_range('0', '9');
    auto letters = lex::Char_set::from_range('a', 'z');

    auto combined = digits | letters;
    TEST_CHECK_EQ(combined.count(), 10 + 26);
    TEST_CHECK(combined.test('3'));
    TEST_CHECK(combined.test('x'));
    TEST_CHECK(!combined.test('A'));

    auto intersection = digits & letters;
    TEST_CHECK(intersection.empty());

    auto inverted = ~digits;
    TEST_CHECK_EQ(inverted.count(), 256 - 10);
    TEST_CHECK(!inverted.test('7'));
    TEST_CHECK(inverted.test('a'));

    auto any_non_nl = lex::Char_set::any_except_newline();
    TEST_CHECK(!any_non_nl.test('\n'));
    TEST_CHECK(any_non_nl.test('\r'));
    TEST_CHECK(any_non_nl.test('a'));
    TEST_CHECK_EQ(any_non_nl.count(), 255);
}

void run_char_set_tests() {
    lex::test::run_test("Char_set Basics", test_char_set_basics);
    lex::test::run_test("Char_set Operations", test_char_set_operations);
}
