#include "aptina_api_loader.hpp"
#include "utils/dynamic_loader/dynamic_library_loader.hpp"
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

static std::unique_ptr<DynamicLibraryLoader> g_aptina_lib;

bool load_aptina_library(const std::string& lib_path) {
    if (g_aptina_lib) {
        return true;
    }
    try {
        auto library = std::make_unique<DynamicLibraryLoader>(lib_path);
        
        ap_Create = library->loadSymbol<ap_Create_t>("ap_Create");
        ap_GrabFrame = library->loadSymbol<ap_GrabFrame_t>("ap_GrabFrame");
        ap_GetLastError = library->loadSymbol<ap_GetLastError_t>("ap_GetLastError");
        ap_Finalize = library->loadSymbol<ap_Finalize_t>("ap_Finalize");
        ap_Destroy = library->loadSymbol<ap_Destroy_t>("ap_Destroy");
        ap_SetState = library->loadSymbol<ap_SetState_t>("ap_SetState");
        ap_LoadIniPreset = library->loadSymbol<ap_LoadIniPreset_t>("ap_LoadIniPreset");
        ap_CheckSensorState = library->loadSymbol<ap_CheckSensorState_t>("ap_CheckSensorState");
        ap_GetImageFormat = library->loadSymbol<ap_GetImageFormat_t>("ap_GetImageFormat");

        g_aptina_lib = std::move(library);
        
        return true;
    } catch (const std::exception& e) {
        g_aptina_lib.reset();
        return false;
    }
}
