#pragma once

#include <cstdint>
#include <deque>
#include <memory>
#include <set>
#include <sstream>
#include <string>
#include <variant>
#include <vector>

#include "symint/classes.hpp"
#include "symint/node_kind.hpp"
#include "symint/int_range.hpp"


namespace symint {


using node_ptr_t = std::shared_ptr<Node>;


inline thread_local std::vector<Node*> DESTRUCTION_QUEUE;
inline thread_local bool DESTRUCTION_QUEUE_GUARD = false;


class Node : public std::enable_shared_from_this<Node> {
protected:
    IntRange _range;
    std::set<std::weak_ptr<Node>, std::owner_less<std::weak_ptr<Node>>> _dependents;
    std::vector<node_ptr_t> _terms;
    std::string _name;
    NodeKind _kind;

    friend class SymInt;

    void clean_dependents() {
        auto weak = weak_from_this();
        for (const auto& t : _terms)
            t->_dependents.erase(weak);
    }

    void recompute() {
        // Save dependents before any assignment that could overwrite them.
        auto saved_dependents = _dependents;

        switch (_kind) {
            case NodeKind::Negation: {
                *this = *(-_terms[0]);
                break;
            }

            case NodeKind::Addition: {
                auto sum = _terms[0] + _terms[1];
                for (int i = 2; i < _terms.size(); ++i)
                    sum = sum + _terms[i];
                *this = *sum;
                break;
            }

            case NodeKind::Subtraction: {
                *this = *(_terms[0] - _terms[1]);
                break;
            }

            case NodeKind::Multiplication: {
                auto prod = _terms[0] * _terms[1];
                for (int i = 2; i < _terms.size(); ++i)
                    prod = prod * _terms[i];
                *this = *prod;
                break;
            }

            case NodeKind::Division: {
                *this = *(_terms[0] / _terms[1]);
                break;
            }

            case NodeKind::FloorDivision: {
                *this = *(floor_div(_terms[0], _terms[1]));
                break;
            }

            case NodeKind::Modulus: {
                *this = *(_terms[0] % _terms[1]);
                break;
            }

            case NodeKind::FloorModulus: {
                *this = *(floor_mod(_terms[0], _terms[1]));
                break;
            }

            default:
                break;
        }
        _dependents = saved_dependents;

        for (auto& d : saved_dependents) {
            auto dep = d.lock();
            if (dep) {
                dep->recompute();
            }
        }
    }

    friend node_ptr_t operator-(const node_ptr_t x);
    friend node_ptr_t operator+(const node_ptr_t x, const node_ptr_t y);
    friend node_ptr_t operator-(const node_ptr_t x, const node_ptr_t y);
    friend node_ptr_t operator*(const node_ptr_t x, const node_ptr_t y);
    friend node_ptr_t operator/(const node_ptr_t x, const node_ptr_t y);
    friend node_ptr_t floor_div(const node_ptr_t x, const node_ptr_t y);
    friend node_ptr_t operator%(const node_ptr_t x, const node_ptr_t y);
    friend node_ptr_t floor_mod(const node_ptr_t x, const node_ptr_t y);

public:
    template<typename ... Args>
    static node_ptr_t make_node(Args&& ... args) {
        auto raw_ptr = new Node(std::forward<Args>(args)...);
        auto ptr = std::shared_ptr<Node>(
            raw_ptr,
            [](Node* p){
                DESTRUCTION_QUEUE.push_back(p);

                if (DESTRUCTION_QUEUE_GUARD)
                    return;

                DESTRUCTION_QUEUE_GUARD = true;

                while (!DESTRUCTION_QUEUE.empty()) {
                    Node* pp = DESTRUCTION_QUEUE.back();
                    DESTRUCTION_QUEUE.pop_back();
                    delete pp;
                }

                DESTRUCTION_QUEUE_GUARD = false;
            }
        );
        return ptr;
    }

    Node() : _kind(NodeKind::Leaf) {}

    Node(std::int64_t value) : _kind(NodeKind::Leaf), _range(value, value) {}

    Node(std::string name, std::int64_t min, std::int64_t max) : _kind(NodeKind::Leaf), _name(name), _range(min, max) {}

    Node(NodeKind kind, std::initializer_list<std::shared_ptr<Node>> terms) : _kind(kind), _terms(terms) {}

    Node(NodeKind kind, const std::vector<std::shared_ptr<Node>>& terms) : _kind(kind) {
        _terms.insert(_terms.end(), std::begin(terms), std::end(terms));
    }

    bool known() const { return _range.is_single_value(); }

    std::int64_t value() const {
        if (!known())
            throw std::runtime_error("Attempting to get the value of an unknown SymInt");
        return _range.min();
    }

    bool is_zero() const { return known() && value() == 0; }

    bool is_one() const { return known() && value() == 1; }

    bool is_minus_one() const { return known() && value() == -1; }

    ~Node() { clean_dependents(); }

    NodeKind kind() const { return _kind; }

    IntRange range() const {
//        if (_stale) {
//            _range = recompute_range();
//            _stale = false;
//        }
        return _range;
    }

    void set_range(IntRange rng) {
        if (rng == _range)
            return;

        _range = rng;
        if (known()) {
            clean_dependents();
            _terms.clear();
            _kind = NodeKind::Leaf;
        }
        recompute();
    }

    /**
     * Needs to be called manually after construction
     */
    void register_dependencies() {
        auto weak = weak_from_this();
        for (const auto& t : _terms)
            t->_dependents.insert(weak);
    }

    const std::vector<node_ptr_t>& terms() const { return _terms; }

    const std::string& name() const { return _name; }

    std::stringstream& expr(std::stringstream& ss) const {
        using Action = std::variant<const Node*, const char*>;
        std::deque<Action> queue;
        queue.push_back(this);

        while (!queue.empty()) {
            Action action = queue.front();
            queue.pop_front();

            if (const char** lit = std::get_if<const char*>(&action)) {
                ss << *lit;
                continue;
            }

            const Node* node = std::get<const Node*>(action);

            if (node->known()) {
                ss << node->value();
                continue;
            }

            if (node->_terms.empty()) {
                ss << (node->_name.empty() ? "?" : node->_name.c_str());
                continue;
            }

            if (node->_kind == NodeKind::Negation) {
                queue.insert(queue.begin(), {"-(", Action{node->_terms[0].get()}, ")"});
                continue;
            }

            const char* op = nullptr;
            switch (node->_kind) {
                case NodeKind::Addition      : op = " + " ; break;
                case NodeKind::Subtraction   : op = " + " ; break;
                case NodeKind::Multiplication: op = "*"   ; break;
                case NodeKind::Division      : op = "/"   ; break;
                case NodeKind::FloorDivision : op = "//"  ; break;
                case NodeKind::Modulus       : op = " % " ; break;
                case NodeKind::FloorModulus  : op = " %% "; break;
                default: op = "?"; break;
            }

            std::vector<Action> actions = {"("};
            for (int i = 0; i < static_cast<int>(node->_terms.size()); ++i) {
                if (i != 0)
                    actions.push_back(op);
                actions.push_back(node->_terms[i].get());
            }
            actions.push_back(")");
            queue.insert(queue.begin(), actions.begin(), actions.end());
        }

        return ss;
    }

    const std::string expr() const {
        std::stringstream ss;
        expr(ss);
        return ss.str();
    }

    std::stringstream& c_code(std::stringstream& ss) const {
        using Action = std::variant<const Node*, const char*>;
        std::deque<Action> queue;
        queue.push_back(this);

        while (!queue.empty()) {
            Action action = queue.front();
            queue.pop_front();

            if (const char** lit = std::get_if<const char*>(&action)) {
                ss << *lit;
                continue;
            }

            const Node* node = std::get<const Node*>(action);

            if (node->known()) {
                ss << node->value();
                continue;
            }

            if (node->_terms.empty()) {
                if (node->_name.empty())
                    throw std::runtime_error("Cannot generate C/C++ code with an anonymous SymInt");
                ss << node->_name;
                continue;
            }

            if (node->_kind == NodeKind::Negation) {
                queue.insert(queue.begin(), {"(-", Action{node->_terms[0].get()}, ")"});
                continue;
            }

            if (node->_kind == NodeKind::FloorDivision) {
                /// ((n - (((n % d) + d) % d)) / d)
                const Node* n = node->_terms[0].get();
                const Node* d = node->_terms[1].get();
                queue.insert(queue.begin(), {
                    "((", Action{n}, " - ", "(((", Action{n}, " % ", Action{d}, ") + ", Action{d}, ") % ", Action{d}, ")) / ", Action{d}, ")"
                });
                continue;
            }

            if (node->_kind == NodeKind::FloorModulus) {
                // (((n % d) + d) % d)
                const Node* n = node->_terms[0].get();
                const Node* d = node->_terms[1].get();
                queue.insert(queue.begin(), {
                    "(((", Action{n}, " % ", Action{d}, ") + ", Action{d}, ") % ", Action{d}, ")"
                });
                continue;
            }

            const char* op = nullptr;
            switch (node->_kind) {
                case NodeKind::Addition      : op = " + "; break;
                case NodeKind::Subtraction   : op = " - "; break;
                case NodeKind::Multiplication: op = "*"  ; break;
                case NodeKind::Division      : op = "/"  ; break;
                case NodeKind::Modulus       : op = " % "; break;
                default: throw;
            }

            std::vector<Action> actions = {"("};
            for (int i = 0; i < static_cast<int>(node->_terms.size()); ++i) {
                if (i != 0)
                    actions.push_back(op);
                actions.push_back(node->_terms[i].get());
            }
            actions.push_back(")");
            queue.insert(queue.begin(), actions.begin(), actions.end());
        }

        return ss;
    }

    const std::string c_code() const {
        std::stringstream ss;
        c_code(ss);
        return ss.str();
    }
};


node_ptr_t operator-(const node_ptr_t x) {
    if (x->known())
        return Node::make_node(-x->value());

    if (x->_kind == NodeKind::Negation)
        return x->_terms[0];

    node_ptr_t r = Node::make_node(NodeKind::Negation, std::vector<node_ptr_t>{x});
    r->register_dependencies();
    r->_range = -x->_range;  // direct setting of the range since it will never be known
    return r;
}


node_ptr_t operator+(const node_ptr_t x, const node_ptr_t y) {
    if (x->known() && y->known())
        return Node::make_node(x->value() + y->value());

    if (x->is_zero())
        return y;

    if (y->is_zero())
        return x;

    std::vector<node_ptr_t> terms;

    auto collect = [&](const node_ptr_t& s) {
        if (s->kind() == NodeKind::Addition) {
            for (const auto& node : s->_terms)
                terms.push_back(node);
        }
        else {
            terms.push_back(s);
        }
    };

    collect(x);
    collect(y);

    auto r = Node::make_node(NodeKind::Addition, terms);
    r->register_dependencies();
    r->_range = x->_range + y->_range;
    return r;
}


node_ptr_t operator-(const node_ptr_t x, const node_ptr_t y) {
    if (x->known() && y->known())
        return Node::make_node(x->value() - y->value());

    if (x->is_zero())
        return -y;

    if (y->is_zero())
        return x;

    if (x.get() == y.get())
        return Node::make_node(0);

    if (y->kind() == NodeKind::Negation)
        return x + y->_terms[0];

    auto r = Node::make_node(NodeKind::Subtraction, std::vector<node_ptr_t>{x, y});
    r->register_dependencies();
    r->_range = x->_range - y->_range;
    return r;
}


node_ptr_t operator*(const node_ptr_t x, const node_ptr_t y) {
    if (x->known() && y->known())
        return Node::make_node(x->value() * y->value());

    if (x->is_zero() || y->is_zero())
        return Node::make_node(0);

    if (x->is_one())
        return y;

    if (y->is_one())
        return x;

    if (x->is_minus_one())
        return -y;

    if (y->is_minus_one())
        return -x;

    std::vector<node_ptr_t> terms;

    auto collect = [&](const node_ptr_t& s) {
        if (s->kind() == NodeKind::Multiplication) {
            for (const auto& node : s->_terms)
                terms.push_back(node);
        }
        else {
            terms.push_back(s);
        }
    };

    collect(x);
    collect(y);

    auto r = Node::make_node(NodeKind::Multiplication, terms);
    r->register_dependencies();
    r->_range = x->_range * y->_range;
    return r;
}


node_ptr_t operator/(const node_ptr_t x, const node_ptr_t y) {
    if (y->is_zero())
        throw std::runtime_error("Division by zero");

    if (x->known() && y->known())
        return Node::make_node(x->value() / y->value());

    if (x->is_zero())
        return Node::make_node(0);

    if (y->is_one())
        return x;

    if (y->is_minus_one())
        return -x;

    auto r = Node::make_node(NodeKind::Division, std::vector<node_ptr_t>{x, y});
    r->register_dependencies();
    r->_range = x->_range / y->_range;
    return r;
}


node_ptr_t floor_div(const node_ptr_t x, const node_ptr_t y) {
    if (y->is_zero())
        throw std::runtime_error("Division by zero");

    if (x->known() && y->known())
        return Node::make_node(saturated_floor_div(x->value(), y->value()));

    if (x->is_zero())
        return Node::make_node(0);

    if (y->is_one())
        return x;

    if (y->is_minus_one())
        return -x;

    if (x->_range.is_nonnegative() && y->_range.is_nonnegative())
        return x / y;

    auto r = Node::make_node(NodeKind::FloorDivision, std::vector<node_ptr_t>{x, y});
    r->register_dependencies();
    r->_range = floor_div(x->_range, y->_range);
    return r;
}


node_ptr_t operator%(const node_ptr_t x, const node_ptr_t y) {
    if (y->is_zero())
        throw std::runtime_error("Division by zero");

    if (x->known() && y->known())
        return Node::make_node(x->value() % y->value());

    if (x->is_zero())
        return Node::make_node(0);

    if (IntRange(-1, 1).contains(y->_range))
        return Node::make_node(0);

    auto r = Node::make_node(NodeKind::Modulus, std::vector<node_ptr_t>{x, y});
    r->register_dependencies();
    r->_range = x->_range % y->_range;
    return r;
}


node_ptr_t floor_mod(const node_ptr_t x, const node_ptr_t y) {
    if (y->is_zero())
        throw std::runtime_error("Division by zero");

    if (x->known() && y->known()) {
        auto n = x->value();
        auto d = y->value();
        return Node::make_node(((n % d) + d) % d);
    }

    if (x->is_zero())
        return Node::make_node(0);

    if (IntRange(-1, 1).contains(y->_range))
        return Node::make_node(0);

    if (x->_range.is_nonnegative() && y->_range.is_nonnegative())
        return x % y;

    auto r = Node::make_node(NodeKind::FloorModulus, std::vector<node_ptr_t>{x, y});
    r->register_dependencies();
    r->_range = floor_mod(x->_range, y->_range);
    return r;
}


std::ostream& operator<<(std::ostream& os, const Node& n) {
    std::stringstream ss;
    return os << n.expr(ss).str();
}


std::ostream& operator<<(std::ostream& os, const node_ptr_t& n) { return os << *n; }


}  // namespace symint
