#pragma once

#include <functional>
#include <memory>
#include <string>
#include <unordered_map>

#include <boost/filesystem.hpp>

#include <core/transport/transport.hpp>




#include "yaml-cpp/yaml.h"

namespace vidre {
    class RuntimeInstance;

    struct LoaderOptions {
        std::string aptinaLibraryPath;
        std::string pylonLibraryPath;
        std::string aptinaBaseLibraryPath;
        std::string pylonBaseLibraryPath;
        TransportMode transportMode = TransportMode::LocalStorage;
        std::string storageEndpoint;
        bool aptinaLibraryPathCliOverride = false;
        bool pylonLibraryPathCliOverride = false;
        bool aptinaBaseLibraryPathCliOverride = false;
        bool pylonBaseLibraryPathCliOverride = false;
    };

    using LoaderLogCallback = std::function<void(const std::string&)>;
    
    std::unordered_map<std::string, std::unique_ptr<vidre::RuntimeInstance>> LoadSystemSetup(
        const std::string& setupFilepath,
        LoaderOptions options = {},
        LoaderLogCallback logMessage = {});
}  // namespace vidre