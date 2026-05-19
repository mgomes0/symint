#include <limits>

#include <gtest/gtest.h>

#include "symint/int_range.hpp"
#include "symint/symint.hpp"


TEST(IntRangeAddition, Basic) {
    symint::IntRange a, b, c;

    a = symint::IntRange(5, 11);
    b = symint::IntRange(7, 13);
    c = a + b;
    EXPECT_EQ(c.min(), 5 + 7);
    EXPECT_EQ(c.max(), 11 + 13);
    c = a - b;
    EXPECT_EQ(c.min(), -8);
    EXPECT_EQ(c.max(), 4);
    c = a / b;
    EXPECT_EQ(c.min(), 5/7);
    EXPECT_EQ(c.max(), 11/7);

    a = symint::IntRange();
    b = symint::IntRange();
    c = a + b;
    EXPECT_EQ(c.min(), -symint::INT_INF);
    EXPECT_EQ(c.max(), symint::INT_INF);
    c = a - b;
    EXPECT_EQ(c.min(), -symint::INT_INF);
    EXPECT_EQ(c.max(), symint::INT_INF);
    c = a / b;
    EXPECT_EQ(c.min(), -symint::INT_INF);
    EXPECT_EQ(c.max(), symint::INT_INF);

    a = symint::IntRange(-5, 1ll << 62);
    b = symint::IntRange(-7, 1ll << 62);
    c = a + b;
    EXPECT_EQ(c.min(), -5 - 7);
    EXPECT_EQ(c.max(), symint::INT_INF);
}






//TEST(IntRangeSubtraction, Basic) {
//    symint::IntRange a(3, 10);
//    symint::IntRange b(1, 4);
//    auto result = a - b;
//    EXPECT_EQ(result.min(), 3 - 4);
//    EXPECT_EQ(result.max(), 10 - 1);
//}
//
//TEST(IntRangeSubtraction, NegativeRange) {
//    symint::IntRange a(-5, 5);
//    symint::IntRange b(-3, 3);
//    auto result = a - b;
//    EXPECT_EQ(result.min(), -5 - 3);
//    EXPECT_EQ(result.max(), 5 - (-3));
//}
//
//TEST(IntRangeSubtraction, SaturatesAtMax) {
//    constexpr auto kMax = symint::INT_INF;
//    constexpr auto kMin = std::numeric_limits<std::int64_t>::min();
//    symint::IntRange a(kMax, kMax);
//    symint::IntRange b(kMin, kMin);
//    auto result = a - b;
//    EXPECT_EQ(result.max(), kMax);
//}
//
//TEST(IntRangeSubtraction, SaturatesAtMin) {
//    constexpr auto kMax = std::numeric_limits<std::int64_t>::max();
//    constexpr auto kMin = std::numeric_limits<std::int64_t>::min();
//    symint::IntRange a(kMin, kMin);
//    symint::IntRange b(kMax, kMax);
//    auto result = a - b;
//    EXPECT_EQ(result.min(), kMin);
//}
//
//
//TEST(IntRangeMultiplication, Basic) {
//    symint::IntRange a(3, 10);
//    symint::IntRange b(2, 5);
//    auto result = a * b;
//    EXPECT_EQ(result.min(), 3 * 2);
//    EXPECT_EQ(result.max(), 10 * 5);
//}
//
//TEST(IntRangeMultiplication, NegativeRange) {
//    symint::IntRange a(-5, 5);
//    symint::IntRange b(-3, 3);
//    auto result = a * b;
//    EXPECT_EQ(result.min(), -5 * 3);
//    EXPECT_EQ(result.max(), 5 * 3);
//}
//
//TEST(IntRangeMultiplication, BothNegative) {
//    symint::IntRange a(-10, -3);
//    symint::IntRange b(-5, -2);
//    auto result = a * b;
//    EXPECT_EQ(result.min(), (-3) * (-2));
//    EXPECT_EQ(result.max(), (-10) * (-5));
//}
//
//TEST(IntRangeMultiplication, MixedSigns) {
//    symint::IntRange a(-4, 6);
//    symint::IntRange b(2, 3);
//    auto result = a * b;
//    EXPECT_EQ(result.min(), -4 * 3);
//    EXPECT_EQ(result.max(), 6 * 3);
//}
//
//TEST(IntRangeMultiplication, SaturatesAtMax) {
//    constexpr auto kMax = std::numeric_limits<std::int64_t>::max();
//    symint::IntRange a(kMax, kMax);
//    symint::IntRange b(2, 2);
//    auto result = a * b;
//    EXPECT_EQ(result.max(), kMax);
//}
//
//TEST(IntRangeMultiplication, SaturatesAtMin) {
//    constexpr auto kMax = std::numeric_limits<std::int64_t>::max();
//    constexpr auto kMin = std::numeric_limits<std::int64_t>::min();
//    symint::IntRange a(kMax, kMax);
//    symint::IntRange b(-2, -2);
//    auto result = a * b;
//    EXPECT_EQ(result.min(), kMin);
//}
//
//
//TEST(IntRangeDivision, BothPositive) {
//    symint::IntRange a(3, 10);
//    symint::IntRange b(2, 5);
//    auto result = a / b;
//    EXPECT_EQ(result.min(), 3 / 5);   // 0
//    EXPECT_EQ(result.max(), 10 / 2);  // 5
//}
//
//TEST(IntRangeDivision, BothNegative) {
//    symint::IntRange a(-10, -3);
//    symint::IntRange b(-5, -2);
//    auto result = a / b;
//    // corners: (-10)/(-5)=2, (-10)/(-2)=5, (-3)/(-5)=0, (-3)/(-2)=1
//    EXPECT_EQ(result.min(), 0);
//    EXPECT_EQ(result.max(), 5);
//}
//
//TEST(IntRangeDivision, MixedNumeratorPositiveDivisor) {
//    symint::IntRange a(-4, 6);
//    symint::IntRange b(2, 3);
//    auto result = a / b;
//    // corners: -4/2=-2, -4/3=-1, 6/2=3, 6/3=2
//    EXPECT_EQ(result.min(), -2);
//    EXPECT_EQ(result.max(), 3);
//}
//
//TEST(IntRangeDivision, DivisorContainsZero) {
//    constexpr auto kMax = std::numeric_limits<std::int64_t>::max();
//    constexpr auto kMin = std::numeric_limits<std::int64_t>::min();
//    symint::IntRange a(1, 10);
//    symint::IntRange b(-1, 1);
//    auto result = a / b;
//    EXPECT_EQ(result.min(), kMin);
//    EXPECT_EQ(result.max(), kMax);
//}
//
//TEST(IntRangeDivision, OverflowMinDivNegOne) {
//    constexpr auto kMax = std::numeric_limits<std::int64_t>::max();
//    constexpr auto kMin = std::numeric_limits<std::int64_t>::min();
//    symint::IntRange a(kMin, kMin);
//    symint::IntRange b(-1, -1);
//    auto result = a / b;
//    EXPECT_EQ(result.min(), kMax);
//    EXPECT_EQ(result.max(), kMax);
//}
//
//
//TEST(IntRangeModulus, BothPositive) {
//    symint::IntRange a(3, 10);
//    symint::IntRange b(2, 5);
//    auto result = a % b;
//    // bound = 4; result in [0, min(10, 4)]
//    EXPECT_EQ(result.min(), 0);
//    EXPECT_EQ(result.max(), 4);
//}
//
//TEST(IntRangeModulus, PositiveDividendBoundedBelowDivisor) {
//    // When x_max < bound the tighter bound from x_max applies.
//    symint::IntRange a(1, 3);
//    symint::IntRange b(5, 10);
//    auto result = a % b;
//    // bound = 9; result in [0, min(3, 9)] = [0, 3]
//    EXPECT_EQ(result.min(), 0);
//    EXPECT_EQ(result.max(), 3);
//}
//
//TEST(IntRangeModulus, BothNegative) {
//    symint::IntRange a(-10, -3);
//    symint::IntRange b(-5, -2);
//    auto result = a % b;
//    // y_abs_max = 5, bound = 4; result in [max(-10,-4), 0] = [-4, 0]
//    EXPECT_EQ(result.min(), -4);
//    EXPECT_EQ(result.max(), 0);
//}
//
//TEST(IntRangeModulus, MixedDividendPositiveDivisor) {
//    symint::IntRange a(-4, 6);
//    symint::IntRange b(2, 3);
//    auto result = a % b;
//    // bound = 2; result in [max(-4,-2), min(6,2)] = [-2, 2]
//    EXPECT_EQ(result.min(), -2);
//    EXPECT_EQ(result.max(), 2);
//}
//
//TEST(IntRangeModulus, DivisorIsOne) {
//    symint::IntRange a(-100, 100);
//    symint::IntRange b(1, 1);
//    auto result = a % b;
//    // any value % 1 == 0
//    EXPECT_EQ(result.min(), 0);
//    EXPECT_EQ(result.max(), 0);
//}
//
//TEST(IntRangeModulus, DivisorIsNegativeOne) {
//    symint::IntRange a(-100, 100);
//    symint::IntRange b(-1, -1);
//    auto result = a % b;
//    // any value % -1 == 0
//    EXPECT_EQ(result.min(), 0);
//    EXPECT_EQ(result.max(), 0);
//}
//
//TEST(IntRangeModulus, DivisorContainsZero) {
//    constexpr auto kMax = std::numeric_limits<std::int64_t>::max();
//    constexpr auto kMin = std::numeric_limits<std::int64_t>::min();
//    symint::IntRange a(1, 10);
//    symint::IntRange b(-1, 1);
//    auto result = a % b;
//    EXPECT_EQ(result.min(), kMin);
//    EXPECT_EQ(result.max(), kMax);
//}
//
//TEST(IntRangeModulus, LargeDivisorSaturates) {
//    constexpr auto kMax = std::numeric_limits<std::int64_t>::max();
//    constexpr auto kMin = std::numeric_limits<std::int64_t>::min();
//    symint::IntRange a(kMin, kMax);
//    symint::IntRange b(kMin, -1);
//    // y_abs_max = saturated_sub(0, kMin) = kMax, bound = kMax - 1
//    auto result = a % b;
//    EXPECT_EQ(result.min(), -(kMax - 1));
//    EXPECT_EQ(result.max(), kMax - 1);
//}
