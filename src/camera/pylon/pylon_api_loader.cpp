#include "pylon_api_loader.hpp"
#include "utils/dynamic_loader/dynamic_library_loader.hpp"
#include <stdexcept>

PylonInitialize_t PylonInitialize = nullptr;
PylonTerminate_t PylonTerminate = nullptr;

static std::unique_ptr<DynamicLibraryLoader> g_pylon_lib;

bool load_pylon_library(const std::string& lib_path) {
    if (g_pylon_lib) return true;
    try {
        auto library = std::make_unique<DynamicLibraryLoader>(lib_path);
        PylonInitialize = library->loadSymbol<PylonInitialize_t>("PylonInitialize");
        PylonTerminate  = library->loadSymbol<PylonTerminate_t>("PylonTerminate");

        g_pylon_lib = std::move(library);
        return true;
    } catch (const std::exception& e) {
        g_pylon_lib.reset();
        return false;
    }
}
