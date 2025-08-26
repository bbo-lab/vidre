#include "pylon_api_loader.hpp"
#include "../../utils/dynamic_library_loader.hpp"
#include <stdexcept>

PylonInitialize_t PylonInitialize = nullptr;
PylonTerminate_t PylonTerminate = nullptr;

static DynamicLibraryLoader* loader = nullptr;

bool load_pylon_library(const std::string& lib_path) {
    if (loader) return true;
    try {
        loader = new DynamicLibraryLoader(lib_path);
        PylonInitialize = loader->loadSymbol<PylonInitialize_t>("PylonInitialize");
        PylonTerminate = loader->loadSymbol<PylonTerminate_t>("PylonTerminate");
        return true;
    } catch (const std::exception& e) {
        delete loader;
        loader = nullptr;
        return false;
    }
}
