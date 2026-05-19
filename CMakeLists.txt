cmake_minimum_required(VERSION 3.18)
project(symint LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# When built via scikit-build-core (i.e. as a Python wheel), SKBUILD is defined.
# In that mode we only build the pybind11 extension; C++ tests are off by default.
if(DEFINED SKBUILD)
    set(_default_build_python ON)
    set(_default_build_cpp_tests OFF)
else()
    set(_default_build_python OFF)
    set(_default_build_cpp_tests ON)
endif()

option(SYMINT_BUILD_PYTHON_BINDINGS "Build pybind11 Python bindings" ${_default_build_python})
option(SYMINT_BUILD_CPP_TESTS "Build C++ unit tests (GoogleTest)" ${_default_build_cpp_tests})

if(SYMINT_BUILD_PYTHON_BINDINGS)
    find_package(Python COMPONENTS Interpreter Development.Module REQUIRED)
    find_package(pybind11 CONFIG REQUIRED)
endif()

add_subdirectory(cpp)

if(SYMINT_BUILD_CPP_TESTS)
    enable_testing()
    add_subdirectory(tests/cpp)
endif()
