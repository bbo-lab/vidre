#include <iostream>

#include "yaml-cpp/yaml.h"
#include "vidre/pylon_camera.hpp"

int main() {
    const char *systemConfigFilepath = "C:/Projects/vidre/config/setupExample.yaml";
    YAML::Node config = YAML::LoadFile(systemConfigFilepath);

    std::unordered_map<std::string, std::unique_ptr<vidre::ICamera>> cameras;

    for (YAML::const_iterator it = config.begin(); it != config.end(); ++it) {
        if (!it->second.IsMap())
            continue;
        if (it->second["type"].as<std::string>().compare("pylon") != 0)
            continue;
        if (!it->second["serial"])
            continue;

        std::string cameraName = it->first.as<std::string>();
        std::cout << cameraName << "\n";
        std::cout << "\taddress: " << it->second["serial"].as<int>() << "\n";
        std::cout << "\tconfig_file: " << it->second["config_file"].as<std::string>() << "\n";

        cameras[cameraName] = std::make_unique<vidre::PylonCamera>();
        cameras[cameraName]->Init();
        if (cameras[cameraName]->LoadConfiguration(it->second["config_file"].as<std::string>().c_str()))
            std::cout << "Config loaded successfully!" << std::endl;

        // if (!it)
        //     continue;
        // if (it->first["type"].as<std::string>().compare("pylon") == 0)
        //     std::cout << it->first.as<std::string>() << std::endl;
        //  std::cout << "Playing at " << it->first.as<std::string>() << " is " << it->second.as<std::string>() << "\n";
    }

    Pylon::PylonTerminate();
    return 0;
}