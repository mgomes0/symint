#pragma once


namespace symint {


enum class NodeKind : std::uint32_t {
    Leaf,
    Negation,
    Addition,
    Subtraction,
    Multiplication,
    Division,
    FloorDivision,
    Modulus,
    FloorModulus,
};

//constexpr std::uint32_t NODE_KIND_LSHIFT = 16;


}  // namespace symint
