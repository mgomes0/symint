#pragma once

#include <algorithm>
#include <cstdint>
#include <limits>

#include "symint/utils.hpp"
#include "symint/classes.hpp"


namespace symint {


class IntRange {
protected:
    std::int64_t _min;
    std::int64_t _max;

    static void _check_min_max(std::int64_t min, std::int64_t max) {
        if (!(
            min <= max &&
            max > -INT_INF &&
            min < INT_INF
        ))
            throw std::runtime_error("Invalid range values (" + std::to_string(min) + ", " + std::to_string(max) + ")");
    }

    friend IntRange operator-(const IntRange& x) {
        IntRange result;
        result._min = -x._max;
        result._max = -x._min;
        return result;
    }

    friend IntRange operator+(const IntRange& x, const IntRange& y) {
        IntRange result;
        result._min = saturated_add(x._min, y._min);
        result._max = saturated_add(x._max, y._max);
        return result;
    }

    friend IntRange operator-(const IntRange& x, const IntRange& y) {
        IntRange result;
        result._min = saturated_sub(x._min, y._max);
        result._max = saturated_sub(x._max, y._min);
        return result;
    }

    friend IntRange operator*(const IntRange& x, const IntRange& y) {
        IntRange result;
        std::int64_t p1 = saturated_mul(x._min, y._min);
        std::int64_t p2 = saturated_mul(x._min, y._max);
        std::int64_t p3 = saturated_mul(x._max, y._min);
        std::int64_t p4 = saturated_mul(x._max, y._max);
        result._min = std::min({p1, p2, p3, p4});
        result._max = std::max({p1, p2, p3, p4});
        return result;
    }

    // Truncating remainder (result has the same sign as the dividend x,
    // matching C++ operator%). If the divisor range contains 0 the result is
    // the full unbounded range.
    friend IntRange operator%(const IntRange& x, const IntRange& y) {
        if (y._min <= 0 && y._max >= 0)
            return IntRange();

        // Maximum absolute value of the divisor (y never contains 0 here).
        std::int64_t y_abs_max = (y._min > 0)
            ? y._max
            : saturated_sub(std::int64_t{0}, y._min);

        // |x % y| < |y|, so the result magnitude is bounded by y_abs_max - 1.
        std::int64_t bound = y_abs_max - 1;

        // Sign of the result follows the sign of x.
        std::int64_t result_min = (x._min >= 0)
            ? std::int64_t{0}
            : std::max(x._min, saturated_sub(std::int64_t{0}, bound));

        std::int64_t result_max = (x._max <= 0)
            ? std::int64_t{0}
            : std::min(x._max, bound);

        IntRange result;
        result._min = result_min;
        result._max = result_max;
        return result;
    }

    // Floor remainder (result has the same sign as the divisor y,
    // matching Python operator%). The result is bounded by the divisor range
    // alone: result ∈ [y_min+1, 0] when y is all-negative, [0, y_max-1] when
    // y is all-positive, and [y_min+1, y_max-1] when y spans zero (with 0
    // excluded from the divisor by contract). Unbounded divisor extremes
    // propagate as unbounded result extremes.
    friend IntRange floor_mod(const IntRange& x, const IntRange& y) {
        std::int64_t result_min = (y._min < 0)
            ? (y._min == -INT_INF ? -INT_INF : y._min + 1)
            : std::int64_t{0};

        std::int64_t result_max = (y._max > 0)
            ? (y._max == INT_INF ? INT_INF : y._max - 1)
            : std::int64_t{0};

        IntRange result;
        result._min = result_min;
        result._max = result_max;
        return result;
    }

    // Truncating division (rounds toward zero, matching C++ operator/).
    // If the divisor range contains 0 the result is the full unbounded range.
    friend IntRange operator/(const IntRange& x, const IntRange& y) {
        if (y._min <= 0 && y._max >= 0)
            return IntRange();
        IntRange result;
        std::int64_t q1 = saturated_div(x._min, y._min);
        std::int64_t q2 = saturated_div(x._min, y._max);
        std::int64_t q3 = saturated_div(x._max, y._min);
        std::int64_t q4 = saturated_div(x._max, y._max);
        result._min = std::min({q1, q2, q3, q4});
        result._max = std::max({q1, q2, q3, q4});
        return result;
    }

//    friend IntRange floor_div(const IntRange& x, const IntRange& y) {
//
//    }

    friend IntRange floor_div(const IntRange& x, const IntRange& y) {
        if (y._min <= 0 && y._max >= 0)
            return IntRange();

        std::int64_t q1 = saturated_floor_div(x._min, y._min);
        std::int64_t q2 = saturated_floor_div(x._min, y._max);
        std::int64_t q3 = saturated_floor_div(x._max, y._min);
        std::int64_t q4 = saturated_floor_div(x._max, y._max);
        IntRange result;
        result._min = std::min({q1, q2, q3, q4});
        result._max = std::max({q1, q2, q3, q4});
        return result;
    }

    friend IntRange abs(const IntRange& x) {
        std::int64_t min = std::abs(x._min);
        std::int64_t max = std::abs(x._max);
        if (min <= max)
            return IntRange(min, max);
        else
            return IntRange(max, min);
    }

    friend bool operator>=(const IntRange& x, std::int64_t y) { return x._min >= y; }

    friend bool operator==(const IntRange& x, const IntRange& y) {
        return x._min == y._min && x._max == y._max;
    }

public:
    IntRange() : _min(-INT_INF), _max(INT_INF) {}

    IntRange(std::int64_t min, std::int64_t max) : _min(min), _max(max) { _check_min_max(min, max); }

    std::int64_t min() const { return _min; }

    std::int64_t max() const { return _max; }

    void set_min(std::int64_t min) {
        _check_min_max(min, _max);
        _min = min;
    }

    void set_max(std::int64_t max) {
        _check_min_max(_min, max);
        _max = max;
    }

    void set_min_max(std::int64_t min, std::int64_t max) {
        _check_min_max(min, max);
        _min = min;
        _max = max;
    }

    bool is_single_value() const {
        return (_min == _max);
    }

    bool value() {
        if (is_single_value())
            return _min;

        throw std::runtime_error("Attempted to get value() from IntRange, but it currently unknown");
    }

    bool contains(const IntRange& other) {
        return _min <= other._min && other._max <= _max;
    }

    bool is_nonnegative() { return _min >= 0; }
};


}  // namespace symint
