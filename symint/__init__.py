"""SymInt: a CAS for integer expressions, powered by a C++ core."""

from ._core import (
    SymInt,
    trunc_div,
    trunc_mod,
)

__version__ = '0.0.1'

__all__ = [
    'SymInt',
    'trunc_div',
    'trunc_mod',
    '__version__',
]
