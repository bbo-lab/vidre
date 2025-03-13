#pragma once

#include <string>
#include <unordered_map>

#include <boost/filesystem.hpp>

#include "vidre/camera.hpp"
#include "vidre/pylon_camera.hpp"
#include "vidre/dummy_camera.hpp"
#include "vidre/runtime.hpp"

#include "yaml-cpp/yaml.h"

namespace vidre {
    std::unordered_map<std::string, std::unique_ptr<vidre::RuntimeInstance>> LoadSystemSetup(std::string setupFilepath);
}  // namespace vidre