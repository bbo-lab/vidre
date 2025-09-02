#pragma once
#include <string>

using PylonInitialize_t = void (*)(void);
using PylonTerminate_t = void (*)(void);

extern PylonInitialize_t PylonInitialize;
extern PylonTerminate_t PylonTerminate;

bool load_pylon_library(const std::string& lib_path);
