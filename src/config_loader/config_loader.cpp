#include "vidre/config_loader.hpp"

namespace vidre {
    std::unique_ptr<vidre::ICamera> LoadCamera(YAML::Node cameraNode) {
        std::unique_ptr<vidre::ICamera> camera;
        if (cameraNode["vendor"] && cameraNode["vendor"].as<std::string>().compare("pylon") == 0) {
            camera = std::make_unique<vidre::PylonCamera>();
            camera->Init();
            if (!camera->LoadConfiguration(cameraNode["config_file"].as<std::string>().c_str()))
                std::cout << "Could not load config file (perhaps wrong path?)." << std::endl;
        }  // TODO: Delegate to Factory
        return camera;
    }

    std::unordered_map<std::string, std::unique_ptr<vidre::ICamera>> LoadSystemSetup(std::string setupFilepath) {
        // Idea for future: Create packages of commands per module from YAML. Afterwards properly distribute
        // over network. Currently just execute those packages locally and just have multiple cameras
        // be able to be started.
        // Example:
        // YAML File has cam1 of type camera:
        // - prepare command with attributes from the YAML file (e.g. what config file to use)
        // YAML File has storage1 of type storage:
        // - ...
        // YAML File has a pipe between cam1 and storage1. Both are on the same node:
        // - (prepare command that tightly couples them (e.g. if two buffers can be replaced by one))

        std::unordered_map<std::string, std::unique_ptr<vidre::ICamera>> cameras;
        if (!boost::filesystem::exists(setupFilepath))
            return cameras;

        YAML::Node config = YAML::LoadFile(setupFilepath);

        for (YAML::const_iterator it = config.begin(); it != config.end(); ++it) {
            if (!it->second.IsMap())
                continue;
            if (!it->second["type"])
                continue;

            if (!it->second["type"].as<std::string>().compare("camera") != 0)
                cameras[it->first.as<std::string>()] = LoadCamera(it->second);
        }

        return cameras;
    }
}  // namespace vidre