#pragma once

#include <string>
#include <unordered_map>

#include <boost/filesystem.hpp>




#include "yaml-cpp/yaml.h"

namespace vidre {
    class RuntimeInstance;
    
    std::unordered_map<std::string, std::unique_ptr<vidre::RuntimeInstance>> LoadSystemSetup(std::string setupFilepath);
}  // namespace vidre