#include <iostream>
#include <string>

#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

#include "dynamic_library_loader.hpp"

namespace vidre {
namespace util {

void* load_library(const std::string& library_name) {
#ifdef _WIN32
    auto result = LoadLibraryExA(library_name.c_str(), NULL, 0);
#else
    dlerror();
    auto result = dlopen(library_name.c_str(), RTLD_LAZY);
#endif
    if (result == nullptr) {
        std::cout << "Failed to load " << library_name << std::endl;
        return nullptr;
    }
    return result;
}

namespace impl{
void* get_function(const std::string& library_name, const std::string& function) {
#ifdef _WIN32
    auto handle = load_library(library_name);
    
    auto result = GetProcAddress((HMODULE)handle, function.c_str());
    if (result == nullptr) {
        std::cout << "Failed to get function" << function << " of library " << library_name << std::endl;
        return nullptr;
    }
#else
    dlerror();

    auto handle = dl_open(library_name);

    auto result = dlsym(handle, function.c_str());
    // dlsym may return NULL even if succeeding
    if (auto error = dlerror()) {
        std::cout << "Failed to get function" << function << " of library " << library_name << std::endl;
        return nullptr;
    }
#endif
    return result;
}
} // namespace impl

} // namespace util
} // namespace vidre