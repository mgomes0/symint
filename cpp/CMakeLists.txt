include(FetchContent)

FetchContent_Declare(
    template-library-extensions
    GIT_REPOSITORY https://github.com/mgomes0/template-library-extentions.git
    GIT_TAG        main
)
FetchContent_MakeAvailable(template-library-extensions)


add_library(symint_core INTERFACE)

target_include_directories(symint_core
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

target_compile_features(symint_core INTERFACE cxx_std_17)

if(SYMINT_BUILD_PYTHON_BINDINGS)
    pybind11_add_module(_core MODULE bindings/bindings.cpp)
    target_link_libraries(_core PRIVATE symint_core)
    target_compile_features(_core PRIVATE cxx_std_17)
    install(TARGETS _core LIBRARY DESTINATION symint)
endif()
