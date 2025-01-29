#pragma once

#include <string>
#include <unordered_map>

#include "vidre/camera.hpp"
#include "vidre/pylon_camera.hpp"
#include "yaml-cpp/yaml.h"

namespace vidre {
    std::unordered_map<std::string, std::unique_ptr<vidre::ICamera>> LoadSystemSetup(std::string setupFilepath);
}  // namespace vidre