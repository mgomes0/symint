#pragma once

namespace symint {
namespace config {
namespace detail {

inline bool g_print_as_c_code = false;

}  // namespace detail

inline void set_print_as_c_code(bool value) {
    detail::g_print_as_c_code = value;
}

inline bool get_print_as_c_code() {
    return detail::g_print_as_c_code;
}

}  // namespace config
}  // namespace symint
