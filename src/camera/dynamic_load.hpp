#pragma once
#include <string>

// Returns true if loaded successfully
bool load_aptina_library(const std::string& lib_path);

// Returns true if loaded successfully
bool load_pylon_library(const std::string& lib_path);
