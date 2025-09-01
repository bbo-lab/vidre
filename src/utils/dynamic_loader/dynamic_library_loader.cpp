#include "dynamic_library_loader.hpp"
#include <iostream>

// Example usage for Aptina
void load_aptina_library(const std::string& lib_path) {
    try {
        DynamicLibraryLoader loader(lib_path);
        // Example: auto init_func = loader.loadSymbol<int(*)(void)>("aptina_init");
        // if (init_func) init_func();
        std::cout << "Aptina library loaded successfully!\n";
    } catch (const std::exception& e) {
        std::cerr << "Failed to load Aptina library: " << e.what() << std::endl;
    }
}

// Example usage for Pylon
void load_pylon_library(const std::string& lib_path) {
    try {
        DynamicLibraryLoader loader(lib_path);
        // Example: auto init_func = loader.loadSymbol<int(*)(void)>("pylon_init");
        // if (init_func) init_func();
        std::cout << "Pylon library loaded successfully!\n";
    } catch (const std::exception& e) {
        std::cerr << "Failed to load Pylon library: " << e.what() << std::endl;
    }
}
