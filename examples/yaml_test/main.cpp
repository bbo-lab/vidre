#include <iostream>

#include "yaml-cpp/yaml.h"

int main() {
    const char *defaultConfigFilepath = "C:\\Projects\\vidre\\config\\setupExample.yaml";
    YAML::Node config = YAML::LoadFile(defaultConfigFilepath);

    for (YAML::const_iterator it = config.begin(); it != config.end(); ++it) {
        if (!it->second.IsMap())
            continue;
        if (it->second["type"].as<std::string>().compare("pylon") != 0)
            continue;

        std::cout << it->first.as<std::string>() << "\n";
        std::cout << "\taddress: " << it->second["address"].as<int>() << "\n";
        std::cout << "\tconfig_file: " << it->second["config_file"].as<std::string>() << "\n";

        // if (!it)
        //     continue;
        // if (it->first["type"].as<std::string>().compare("pylon") == 0)
        //     std::cout << it->first.as<std::string>() << std::endl;
        //  std::cout << "Playing at " << it->first.as<std::string>() << " is " << it->second.as<std::string>() << "\n";
    }

    return 0;
}