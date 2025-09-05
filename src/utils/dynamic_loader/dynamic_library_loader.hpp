#pragma once
#include <string>
#include <stdexcept>

#include "dylib.hpp"

// #ifdef _WIN32
// #include <windows.h>
// #else
// #include <dlfcn.h>
// #endif


class DynamicLibraryLoader {
public:
    DynamicLibraryLoader(const std::string& path) {
        lib = dylib(path);
    }

    ~DynamicLibraryLoader() = default;

    dylib lib = dylib("");
};
