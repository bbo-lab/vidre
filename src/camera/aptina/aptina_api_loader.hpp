#pragma once
#include <string>
#include <cstdint>

using ap_handle_t = void*;

using ap_GrabFrame_t        = unsigned int (*)(ap_handle_t, unsigned char*, unsigned int);
using ap_Create_t           = ap_handle_t (*)(int);
using ap_Destroy_t          = void (*)(ap_handle_t);
using ap_Finalize_t         = void (*)(void);
using ap_LoadIniPreset_t    = int (*)(ap_handle_t, const char*, const char*);
using ap_CheckSensorState_t = int (*)(ap_handle_t, unsigned int);
using ap_SetState_t         = int (*)(ap_handle_t, const char*, int);
using ap_GetImageFormat_t   = int (*)(ap_handle_t, unsigned int*, unsigned int*, char*, unsigned int);
using ap_GetLastError_t     = int (*)(void);
// using ap_GetFrameData_t     = std::uint64_t (*)(ap_handle_t, const char* key);

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
