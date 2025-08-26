#include "aptina_api_loader.hpp"
#include "../../utils/dynamic_library_loader.hpp"
#include <stdexcept>

ap_Create_t ap_Create = nullptr;
ap_GrabFrame_t ap_GrabFrame = nullptr;
ap_GetLastError_t ap_GetLastError = nullptr;
ap_Finalize_t ap_Finalize = nullptr;
ap_Destroy_t ap_Destroy = nullptr;
ap_SetState_t ap_SetState = nullptr;
ap_LoadIniPreset_t ap_LoadIniPreset = nullptr;
ap_CheckSensorState_t ap_CheckSensorState = nullptr;
ap_GetImageFormat_t ap_GetImageFormat = nullptr;

static DynamicLibraryLoader* loader = nullptr;

bool load_aptina_library(const std::string& lib_path) {
    if (loader) return true;
    try {
        loader = new DynamicLibraryLoader(lib_path);
        ap_Create = loader->loadSymbol<ap_Create_t>("ap_Create");
        ap_GrabFrame = loader->loadSymbol<ap_GrabFrame_t>("ap_GrabFrame");
        ap_GetLastError = loader->loadSymbol<ap_GetLastError_t>("ap_GetLastError");
        ap_Finalize = loader->loadSymbol<ap_Finalize_t>("ap_Finalize");
        ap_Destroy = loader->loadSymbol<ap_Destroy_t>("ap_Destroy");
        ap_SetState = loader->loadSymbol<ap_SetState_t>("ap_SetState");
        ap_LoadIniPreset = loader->loadSymbol<ap_LoadIniPreset_t>("ap_LoadIniPreset");
        ap_CheckSensorState = loader->loadSymbol<ap_CheckSensorState_t>("ap_CheckSensorState");
        ap_GetImageFormat = loader->loadSymbol<ap_GetImageFormat_t>("ap_GetImageFormat");
        return true;
    } catch (const std::exception& e) {
        delete loader;
        loader = nullptr;
        return false;
    }
}
