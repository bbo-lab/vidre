#include <iostream>
#include <string>

#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

#include <utils/dynamic_loader/dynamic_library_loader.hpp>

namespace vidre {
namespace util {

    void* LoadLibrary(const std::string& library_name);

    bool IsLibraryAvailable(const std::string& library_name) {
        return LoadLibrary(library_name) != nullptr;
    }

    void* LoadLibrary(const std::string& library_name) {
#ifdef _WIN32
        auto result = LoadLibraryExA(library_name.c_str(), NULL, LOAD_WITH_ALTERED_SEARCH_PATH);
#else
        dlerror();
        auto result = dlopen(library_name.c_str(), RTLD_LAZY);
#endif
        if (result == nullptr) {
            std::cout << "Failed to load " << library_name << std::endl;
            return nullptr;
        }

        std::cout << "Library " << library_name << " is available " << std::endl;
        return result;
    }

namespace impl{
    void* GetFunction(const std::string& library_name, const std::string& function) {
#ifdef _WIN32
        auto handle = LoadLibrary(library_name);
        if (handle == nullptr) {
            return nullptr;
        }

        auto result = GetProcAddress(static_cast<HMODULE>(handle), function.c_str());
        if (result == nullptr) {
            std::cout << "Failed to get function " << function << " of library " << library_name << std::endl;
        }
        return reinterpret_cast<void*>(result);
#else
        auto handle = LoadLibrary(library_name);
        if (handle == nullptr) {
            return nullptr;
        }

        dlerror();
        auto result = dlsym(handle, function.c_str());
        if (auto error = dlerror()) {
            std::cout << "Failed to get function" << function << " of library " << library_name << std::endl;
            (void)error;
            return nullptr;
        }
        return result;
#endif
    }
} // namespace impl

} // namespace util
} // namespace vidre
