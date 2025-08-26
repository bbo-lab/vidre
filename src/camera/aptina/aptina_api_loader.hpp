#pragma once
#include <string>
#include <cstdint>

extern "C" {
    typedef void* (*ap_Create_t)(int);
    typedef int (*ap_GrabFrame_t)(void*, void*, int);
    typedef int (*ap_GetLastError_t)();
    typedef void (*ap_Finalize_t)();
    typedef void (*ap_Destroy_t)(void*);
    typedef int (*ap_SetState_t)(void*, const char*, int);
    typedef int (*ap_LoadIniPreset_t)(void*, const char*, const char*);
    typedef int (*ap_CheckSensorState_t)(void*, int);
    typedef int (*ap_GetImageFormat_t)(void*, unsigned int*, unsigned int*, void*, int);
}

extern ap_Create_t ap_Create;
extern ap_GrabFrame_t ap_GrabFrame;
extern ap_GetLastError_t ap_GetLastError;
extern ap_Finalize_t ap_Finalize;
extern ap_Destroy_t ap_Destroy;
extern ap_SetState_t ap_SetState;
extern ap_LoadIniPreset_t ap_LoadIniPreset;
extern ap_CheckSensorState_t ap_CheckSensorState;
extern ap_GetImageFormat_t ap_GetImageFormat;

bool load_aptina_library(const std::string& lib_path);
