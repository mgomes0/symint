#include <optional>
#include <sstream>

#include <pybind11/pybind11.h>
#include <pybind11/operators.h>
#include <pybind11/stl.h>

#include "symint/symint.hpp"

namespace py = pybind11;


PYBIND11_MODULE(_core, m) {
    m.doc() = "SymInt C++ core bindings";

//    py::enum_<symint::NodeKind>(m, "NodeKind")
//        .value("Constant",          symint::NodeKind::Constant)
//        .value("AnonymousVariable", symint::NodeKind::AnonymousVariable)
//        .value("NamedVariable",     symint::NodeKind::NamedVariable)
//        .value("Addition",          symint::NodeKind::Addition)
//        .value("Subtraction",       symint::NodeKind::Subtraction)
//        .value("Multiplication",    symint::NodeKind::Multiplication)
//        .value("Division",          symint::NodeKind::Division)
//        .value("FloorDivision",     symint::NodeKind::FloorDivision)
//        .value("Modulus",           symint::NodeKind::Modulus)
//        .value("FloorModulus",      symint::NodeKind::FloorModulus)
//        .value("Negation",          symint::NodeKind::Negation)
//        .export_values();

    py::class_<symint::SymInt>(m, "SymInt")
        .def(py::init<>())
        .def(py::init<std::int64_t>(), py::arg("value"))
        .def(
            py::init([](std::string name, std::optional<std::int64_t> min, std::optional<std::int64_t> max) {
                return symint::SymInt(name,
                    min.value_or(-symint::INT_INF),
                    max.value_or(symint::INT_INF));
            }),
            py::arg("name"),
            py::arg("min") = py::none(),
            py::arg("max") = py::none()
        )
        .def(+py::self)
        .def(-py::self)
        .def(py::self + py::self)
        .def(std::int64_t() + py::self)
        .def(py::self - py::self)
        .def(std::int64_t() - py::self)
        .def(py::self * py::self)
        .def(std::int64_t() * py::self)
        .def(py::self / py::self)
        .def(std::int64_t() / py::self)
        .def("__floordiv__", [](const symint::SymInt self, const symint::SymInt other){ return symint::floor_div(self, other); })
        .def("__rfloordiv__", [](const symint::SymInt self, const symint::SymInt other){ return symint::floor_div(other, self); })
        .def("__mod__", [](const symint::SymInt x, const symint::SymInt y){ return symint::floor_mod(x, y); })
        .def("__rmod__", [](const symint::SymInt self, const symint::SymInt other){ return symint::floor_mod(other, self); })
        .def("__int__", [](const symint::SymInt& i) { return i.value(); })
        .def("__repr__", [](const symint::SymInt& i) { return i.expr(); })
        .def("c_code", &symint::SymInt::c_code)
//        .def_property_readonly("kind", &symint::SymInt::kind)
        .def_property(
            "range",
            [](const symint::SymInt& self) {
                auto rng = self.range();
                py::list out;

                if (rng.min() == -symint::INT_INF)
                    out.append(py::none());
                else
                    out.append(rng.min());

                if (rng.max() == symint::INT_INF)
                    out.append(py::none());
                else
                    out.append(rng.max());

                return out.cast<py::tuple>();
            },
            [](symint::SymInt& self, py::tuple tpl){
                std::int64_t lo = tpl[0].is_none() ? -symint::INT_INF : tpl[0].cast<std::int64_t>();
                std::int64_t hi = tpl[1].is_none() ?  symint::INT_INF : tpl[1].cast<std::int64_t>();
                self.set_range(symint::IntRange(lo, hi));
            }
        )
        ;

    py::implicitly_convertible<std::int64_t, symint::SymInt>();

    m.def("trunc_div", [](const symint::SymInt x, const symint::SymInt y) {
        return x / y;
    }, "C-style integer division operation between SymInt objects", py::arg("x"), py::arg("y"));

    m.def("trunc_mod", [](const symint::SymInt x, const symint::SymInt y) {
        return x % y;
    }, "C-style modulus operation between SymInt objects", py::arg("x"), py::arg("y"));


    py::module_ config_m = m.def_submodule("config", "SymInt global configuration");
    config_m.def("set_print_as_c_code", &symint::config::set_print_as_c_code,
                 "Set whether expressions print as C code instead of math notation",
                 py::arg("value"));
    config_m.def("get_print_as_c_code", &symint::config::get_print_as_c_code,
                 "Return whether expressions print as C code instead of math notation");
}
