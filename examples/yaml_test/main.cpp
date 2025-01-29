#include <iostream>

#include <string>
#include <unordered_map>

#include "vidre/config_loader.hpp"
#include "vidre/camera.hpp"

int main() {
    std::string systemConfigFilepath = "C:/Projects/vidre/config/setupExample.yaml";

    std::unordered_map<std::string, std::unique_ptr<vidre::ICamera>> cameras = vidre::LoadSystemSetup(systemConfigFilepath);
    for (auto it = cameras.begin(); it != cameras.end(); ++it) {
        std::cout << it->first << " was " << (it->second ? "" : "not ") << "constructed.\n";
    }
    std::cout.flush();

    return 0;
}