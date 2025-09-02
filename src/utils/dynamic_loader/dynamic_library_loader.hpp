#pragma once
#include <string>
#include <stdexcept>

#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif


class DynamicLibraryLoader {
public:
    DynamicLibraryLoader(const std::string& path) {
#ifdef _WIN32
        handle = LoadLibraryA(path.c_str());
#else
        handle = dlopen(path.c_str(), RTLD_LAZY);
#endif
        if (!handle) {
            throw std::runtime_error(dlerror());
        }
    }

    ~DynamicLibraryLoader() {
        if (handle) {
    #ifdef _WIN32
            FreeLibrary(handle);
    #else
            dlclose(handle);
    #endif
        }
    }

    template<typename T>
    T loadSymbol(const std::string& symbol) {
#ifdef _WIN32
        T sym = reinterpret_cast<T>(GetProcAddress(handle, symbol.c_str()));
#else
        T sym = reinterpret_cast<T>(dlsym(handle, symbol.c_str()));
#endif
       return sym;
    }

private:
#if _WIN32
		HMODULE handle{ nullptr };
#else
		void* handle = nullptr;
#endif
};
