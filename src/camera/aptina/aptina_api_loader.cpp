#include "aptina_api_loader.hpp"
#include "utils/dynamic_loader/dynamic_library_loader.hpp"
#include <stdexcept>
#include <iostream> 

 ap_Create_t* ap_Create = nullptr;
 ap_GrabFrame_t* ap_GrabFrame = nullptr;
 ap_GetLastError_t* ap_GetLastError = nullptr;
 ap_Finalize_t* ap_Finalize = nullptr;
 ap_Destroy_t* ap_Destroy = nullptr;
 ap_SetState_t* ap_SetState = nullptr;
 ap_LoadIniPreset_t* ap_LoadIniPreset = nullptr;
 ap_CheckSensorState_t* ap_CheckSensorState = nullptr;
 ap_GetImageFormat_t* ap_GetImageFormat = nullptr;


bool load_aptina_library(const std::string& lib_path) {
    try {
        dylib lib(lib_path, dylib::no_filename_decorations);
        std::cout << ap_Create << std::endl;
        ap_Create = lib.get_function<ap_Create_t>("ap_Create");
        ap_GrabFrame = lib.get_function<ap_GrabFrame_t>("ap_GrabFrame");
        ap_GetLastError = lib.get_function<ap_GetLastError_t>("ap_GetLastError");
        ap_Finalize = lib.get_function<ap_Finalize_t>("ap_Finalize");
        ap_Destroy = lib.get_function<ap_Destroy_t>("ap_Destroy");
        ap_SetState = lib.get_function<ap_SetState_t>("ap_SetState");
        ap_LoadIniPreset = lib.get_function<ap_LoadIniPreset_t>("ap_LoadIniPreset");
        ap_CheckSensorState = lib.get_function<ap_CheckSensorState_t>("ap_CheckSensorState");
        ap_GetImageFormat = lib.get_function<ap_GetImageFormat_t>("ap_GetImageFormat");

        return true;
    } catch (const std::exception& e) {
        std::cerr << "An exception occurred." << std::endl << e.what() << std::endl;
        return false;
    }
}
