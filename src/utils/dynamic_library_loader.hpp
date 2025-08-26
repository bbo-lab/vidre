#pragma once
#include <string>
#include <dlfcn.h>
#include <stdexcept>

class DynamicLibraryLoader {
public:
    DynamicLibraryLoader(const std::string& path) : handle(nullptr) {
        handle = dlopen(path.c_str(), RTLD_LAZY);
        if (!handle) {
            throw std::runtime_error(dlerror());
        }
    }
    ~DynamicLibraryLoader() {
        if (handle) dlclose(handle);
    }
    template<typename T>
    T loadSymbol(const std::string& symbol) {
        dlerror(); // Clear any existing error
        T sym = reinterpret_cast<T>(dlsym(handle, symbol.c_str()));
        const char* dlsym_error = dlerror();
        if (dlsym_error) {
            throw std::runtime_error(dlsym_error);
        }
        return sym;
    }
private:
    void* handle;
};
