#pragma once


namespace symint {


constexpr std::int64_t INT_INF = std::numeric_limits<std::int64_t>::max();


std::int64_t saturated_add(std::int64_t x, std::int64_t y) {
    if (y > 0 && x > INT_INF - y)
        return INT_INF;
    if (y < 0 && x < -INT_INF - y)
        return -INT_INF;
    return x + y;
}


std::int64_t saturated_sub(std::int64_t x, std::int64_t y) {
    if (y < 0 && x > INT_INF + y)
        return INT_INF;
    if (y > 0 && x < -INT_INF + y)
        return -INT_INF;
    return x - y;
}


std::int64_t saturated_div(std::int64_t x, std::int64_t y) {
    if (x == INT_INF || x == -INT_INF) {
        if (y < 0)
            return -x;
        return x;
    }
    return x / y;
}


std::int64_t saturated_mul(std::int64_t x, std::int64_t y) {
    if (x == 0 || y == 0)
        return 0;

    std::int64_t x_abs = std::abs(x);
    std::int64_t y_abs = std::abs(y);
    std::int64_t x_sgn = x / x_abs;
    std::int64_t y_sgn = y / y_abs;

    if (x_abs >= INT_INF/y_abs && y_abs >= INT_INF/x_abs)
        return x_sgn * y_sgn * INT_INF;

    return x * y;
}


std::int64_t saturated_floor_div(std::int64_t n, std::int64_t d) {
    // floor_div(n, d) = floor(n / d), rounds toward -inf.
    // Implemented as: (n - floor_mod(n, d)) / d
    std::int64_t mod = ((n % d) + d) % d;
    std::int64_t q = (n - mod) / d;
    return q;
}


std::int64_t floor_mod(std::int64_t n, std::int64_t d) {
    return (n % d + d) % d;
}


}  // namespace symint
