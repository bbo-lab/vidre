#pragma once

#include <string>



namespace vidre {
    namespace util {
        bool isLibraryAvailable(const std::string& library_name) {
        #ifdef _WIN32
            auto result = LoadLibraryExA(library_name.c_str(), NULL, 0);
        #else
            dlerror();
            auto result = dlopen(library_name.c_str(), RTLD_LAZY);
        #endif
            if (result == nullptr) {
                std::cout << "Failed to load " << library_name << std::endl;
                return false;
            }

            std::cout << "Library " << library_name << "is available" << std::endl;
            return true;
        }
    
    } // namespace util
} // namespace vidre