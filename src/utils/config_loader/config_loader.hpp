#pragma once

#include <functional>
#include <memory>
#include <string>
#include <unordered_map>

namespace vidre {
    class RuntimeInstance;

    struct LoadSetupOptions {
        std::string aptinaLibraryPath;
        std::string pylonLibraryPath;
        std::string aptinaBaseLibraryPath;
        std::string pylonBaseLibraryPath;
        bool aptinaLibraryPathCliOverride = false;
        bool pylonLibraryPathCliOverride = false;
        bool aptinaBaseLibraryPathCliOverride = false;
        bool pylonBaseLibraryPathCliOverride = false;
        std::function<void(const std::string&)> log;
    };

    std::unordered_map<std::string, std::unique_ptr<vidre::RuntimeInstance>>
    LoadSystemSetup(const std::string& setupFilepath, LoadSetupOptions& options);
}  // namespace vidre
