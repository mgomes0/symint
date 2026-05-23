#pragma once

#include <cstdint>
#include <initializer_list>
#include <iostream>
#include <limits>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "symint/node_kind.hpp"
#include "symint/classes.hpp"
#include "symint/int_range.hpp"
#include "symint/node.hpp"
#include "symint/config.hpp"


namespace symint {


class SymInt {
protected:
    mutable std::shared_ptr<Node> _node_ptr;

    SymInt(std::shared_ptr<Node> node) : _node_ptr(node) {}

    SymInt(NodeKind kind, std::initializer_list<std::shared_ptr<Node>> terms) : _node_ptr(std::make_shared<Node>(kind, terms)) {}

    SymInt(NodeKind kind, const std::vector<std::shared_ptr<Node>>& terms) : _node_ptr(std::make_shared<Node>(kind, terms)) {}

    friend SymInt operator+(const SymInt x);

    friend SymInt operator-(const SymInt x);

    friend SymInt operator+(const SymInt x, const SymInt y);

    friend SymInt operator-(const SymInt x, const SymInt y);

    friend SymInt operator*(const SymInt x, const SymInt y);

    friend SymInt operator/(const SymInt x, const SymInt y);

    friend SymInt floor_div(const SymInt x, const SymInt y);

    friend SymInt operator%(const SymInt x, const SymInt y);

    friend SymInt floor_mod(const SymInt x, const SymInt y);

    friend std::ostream& operator<<(std::ostream& os, const SymInt& i);

public:
    SymInt() : _node_ptr(Node::make_node()) {}

    SymInt(std::int64_t value) : _node_ptr(Node::make_node(value)) {}

    SymInt(std::string name, std::int64_t min = -INT_INF, std::int64_t max = INT_INF) : _node_ptr(Node::make_node(name, min, max)) {}

    NodeKind kind() const { return _node_ptr->kind(); }

    bool known() const { return _node_ptr->known(); }

    std::int64_t value() const { return _node_ptr->value(); }

    bool is_zero() const { return known() && value() == 0; }

    bool is_one() const { return known() && value() == 1; }

    const std::string expr() const { return _node_ptr->expr(); }

    const std::string c_code() const { return _node_ptr->c_code(); }

    IntRange range() const { return _node_ptr->range(); }

    void set_range(const IntRange& new_rng) {
        if (_node_ptr->kind() != NodeKind::Leaf)
            throw std::runtime_error("Setting new range can only be done on Leaf nodes.");

        if (!_node_ptr->_range.contains(new_rng))
            throw std::runtime_error("Setting new range on a value requires that it is contained by the original range.");

        _node_ptr->set_range(new_rng);  // already marks stale
    }
};


SymInt operator+(const SymInt x) {
    return x;
}


SymInt operator-(const SymInt x) { return -x._node_ptr; }

SymInt operator+(const SymInt x, const SymInt y) { return x._node_ptr + y._node_ptr; }


SymInt operator-(const SymInt x, const SymInt y) { return x._node_ptr - y._node_ptr; }


SymInt operator*(const SymInt x, const SymInt y) { return x._node_ptr * y._node_ptr; }


SymInt operator/(const SymInt x, const SymInt y) { return x._node_ptr / y._node_ptr; }


SymInt floor_div(const SymInt x, const SymInt y) { return floor_div(x._node_ptr, y._node_ptr); }


SymInt operator%(const SymInt x, const SymInt y) { return x._node_ptr % y._node_ptr; }


SymInt floor_mod(const SymInt x, const SymInt y) { return floor_mod(x._node_ptr, y._node_ptr); }


std::ostream& operator<<(std::ostream& os, const SymInt& i) { return os << i._node_ptr; }


}  // namespace symint
