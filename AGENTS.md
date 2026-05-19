# AGENTS.md

Guidance for coding agents (and humans) working in this repository.

## Overview

**SymInt** is a simple Computer Algebra System (CAS) for integer expressions,
with an emphasis on code generation. The core is written in C++ (C++17) and
exposed to Python via pybind11.

## Layout

```
.
├── CMakeLists.txt           # top-level; SKBUILD-aware
├── pyproject.toml           # scikit-build-core + pybind11; pytest config
├── cpp/                     # all C++ source
│   ├── include/symint/      # public headers (namespace symint)
│   ├── src/                 # core library implementation
│   └── bindings/            # pybind11 glue → symint._core
├── symint/                  # top-level Python package
│   └── __init__.py          # re-exports from symint._core
└── tests/                   # centralized tests
    ├── test_*.py            # pytest suite (Python API)
    └── cpp/                 # GoogleTest suite (C++ core)
```

## Build & test commands

Python package + pytest (exercises C++ via the bindings).

With [uv](https://docs.astral.sh/uv/) (preferred; a `uv.lock` is checked in):

```bash
uv sync --group dev   # builds the C++ extension and installs test deps into .venv
uv run pytest
```

With pip:

```bash
pip install -e ".[test]"
pytest
```

C++ unit tests (GoogleTest, fetched via CMake `FetchContent`):

```bash
cmake -S . -B build
cmake --build build -j
ctest --test-dir build --output-on-failure
```

When invoked through scikit-build-core (i.e. building the wheel), only the
pybind11 extension is built; C++ tests are off by default in that mode. When
invoked directly via `cmake -S . -B build`, the C++ tests are on by default
and the pybind11 extension is off. Both can be toggled explicitly with
`-DSYMINT_BUILD_PYTHON_BINDINGS=ON/OFF` and `-DSYMINT_BUILD_CPP_TESTS=ON/OFF`.

## Conventions

- **Language**: C++17, no compiler extensions.
- **C++ namespace**: everything goes under `namespace symint { ... }`.
- **C++ classes**: all classes have their data member `protected` prefixed with `_` at the top of the class, public member functions follow after that.
- **Public headers**: `cpp/include/symint/…`, included as `#include "symint/foo.hpp"`.
- **Python package**: top-level `symint/`. The compiled extension is
  `symint._core`; users only ever `import symint`.
- **Tests**: live exclusively under `tests/`. Python tests directly in
  `tests/`, C++ tests under `tests/cpp/`. There is no in-source test code.

## Adding new C++ functionality

End-to-end checklist for a new function/class:

1. **Declare** it in a header under `cpp/include/symint/` (inside `namespace symint`).
2. **Implement** it in a `.cpp` under `cpp/src/` and add the file to the
   `symint_core` target in `cpp/CMakeLists.txt`.
3. **Bind** it in `cpp/bindings/bindings.cpp` so it appears on the `_core` module.
4. **Re-export** it from `symint/__init__.py` so it is reachable as
   `symint.<name>`.
5. **Test** it with both a GoogleTest case in `tests/cpp/` and a pytest case
   in `tests/`. The C++ test guards the core; the Python test guards the
   binding and the public API.

## Do not commit

All changes are to be reviewed, don't commit the code.
