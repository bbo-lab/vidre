#pragma once
#include <string>

extern "C" {
    typedef void (*PylonInitialize_t)();
    typedef void (*PylonTerminate_t)();
}

extern PylonInitialize_t PylonInitialize;
extern PylonTerminate_t PylonTerminate;

bool load_pylon_library(const std::string& lib_path);
