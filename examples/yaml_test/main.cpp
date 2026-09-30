#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

#include <boost/program_options.hpp>

#include <core/runtime/runtime.hpp>
#include <utils/dynamic_loader/dynamic_library_loader.hpp>

#include <boost/filesystem.hpp>




#include <unordered_map>
#include "yaml-cpp/yaml.h"
#include <filesystem>
namespace po = boost::program_options;

// std::string systemConfigFilepath = "C:/Not/A/Real/Path.yaml";
// std::string systemConfigFilepath = "C:/Projects/vidre/config/setupExample.yaml";
std::string systemConfigFilepath = "C:/Users/Almukhamedov_la/Desktop/work/testing2/vidre/config/setupExample.yaml";

std::string aptinaBaseLibraryPath;
std::string pylonBaseLibraryPath;
std::string aptinaPluginLibraryPath;
std::string pylonPluginLibraryPath;

bool aptinaBaseLibraryPathCliOverride = false;
bool pylonBaseLibraryPathCliOverride = false;
bool aptinaPluginLibraryPathCliOverride = false;
bool pylonPluginLibraryPathCliOverride = false;

static std::vector<std::string> split(const char *str, char c = ' ') {
    std::vector<std::string> result;

    do {
        const char *begin = str;

        while (*str != c && *str)
            str++;

        result.push_back(std::string(begin, str));
    } while (0 != *str++);

    return result;
}


std::unique_ptr<vidre::ICamera> LoadCamera(YAML::Node cameraNode) {
    vidre::APTINA_LIBRARY_PATH = aptinaPluginLibraryPath;
    vidre::PYLON_LIBRARY_PATH = pylonPluginLibraryPath;

    if (!cameraNode["vendor"])
        return nullptr;

    // Vendor is specified
    std::unique_ptr<vidre::ICamera> camera;
    if (cameraNode["vendor"].as<std::string>().compare("pylon") == 0
        && (pylonBaseLibraryPath.empty() || vidre::util::IsLibraryAvailable(pylonBaseLibraryPath))) {
        if (vidre::PYLON_LIBRARY_PATH.empty()) {
            std::cout << "Pylon plugin library path is empty. Set it in config or via --pylon-library." << std::endl;
            return nullptr;
        }
        using camera_ref_t = vidre::ICamera*();
        using destroy_ref_t = void(vidre::ICamera*);
        auto create = vidre::util::GetFunction<camera_ref_t*>(vidre::PYLON_LIBRARY_PATH, "create_pylon_camera");
        auto destroy = vidre::util::GetFunction<destroy_ref_t*>(vidre::PYLON_LIBRARY_PATH, "destroy_pylon_camera");
        if (create == nullptr || destroy == nullptr) {
            return nullptr;
        }
        camera = std::unique_ptr<vidre::ICamera>(create());
    }
    else if (cameraNode["vendor"].as<std::string>().compare("aptina") == 0
             && (aptinaBaseLibraryPath.empty() || vidre::util::IsLibraryAvailable(aptinaBaseLibraryPath))) {
        if (vidre::APTINA_LIBRARY_PATH.empty()) {
            std::cout << "Aptina plugin library path is empty. Set it in config or via --aptina-library." << std::endl;
            return nullptr;
        }
        using camera_ref_t = vidre::ICamera*();
        using destroy_ref_t = void(vidre::ICamera*);
        auto create = vidre::util::GetFunction<camera_ref_t*>(vidre::APTINA_LIBRARY_PATH, "create_aptina_camera");
        auto destroy = vidre::util::GetFunction<destroy_ref_t*>(vidre::APTINA_LIBRARY_PATH, "destroy_aptina_camera");
        if (create == nullptr || destroy == nullptr) {
            return nullptr;
        }
        camera = std::unique_ptr<vidre::ICamera>(create());
    }
    else {
        return nullptr;
    }
    
    if (cameraNode["serial"] && !camera->Init(cameraNode["serial"].as<std::string>())) {
        std::cout << "Could not initialize with serial number " << cameraNode["serial"].as<std::string>() << std::endl;
        return nullptr;
    }
    std::cout << "CREATED CAM\n";
    if (!camera->LoadConfiguration(cameraNode["config_file"].as<std::string>().c_str())) {
        std::cout << "Could not load config file (perhaps wrong path?)." << std::endl;
        return nullptr;
    }  // TODO: Delegate to Factory

    // camera->SaveConfiguration("./" + cameraNode["serial"].as<std::string>());
    if (cameraNode["trigger_in"]) {
        try {
            camera->SetTriggerIn(cameraNode["trigger_in"].as<int>());
            camera->triggerInSet = true;
        } catch (const std::exception& e) {
            std::cout << "WARNING: Trigger in could not be set: " << e.what() << std::endl;
        } catch (...) {
            std::cout << "WARNING: Trigger in could not be set" << std::endl;
        }
    }
    if (cameraNode["trigger_out"]) {
        try {
            camera->SetTriggerOut(cameraNode["trigger_out"].as<int>());
            camera->triggerOutSet = true;
        } catch (const std::exception& e) {
            std::cout << "WARNING: Trigger out could not be set: " << e.what() << std::endl;
        } catch (...) {
            std::cout << "WARNING: Trigger out could not be set" << std::endl;
        }
    }

    return camera;
}

std::unordered_map<std::string, std::unique_ptr<vidre::RuntimeInstance>> LoadSystemSetup(std::string setupFilepath) {
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

    std::unordered_map<std::string, std::unique_ptr<vidre::RuntimeInstance>> cameras;
    if (!boost::filesystem::exists(setupFilepath))
        return cameras;

    YAML::Node config = YAML::LoadFile(setupFilepath);

    auto readStringByKeys = [](const YAML::Node& node, const std::vector<std::string>& keys) -> std::string {
        if (!node || !node.IsMap()) {
            return "";
        }
        for (const auto& key : keys) {
            if (node[key] && node[key].IsScalar()) {
                return node[key].as<std::string>();
            }
        }
        return "";
    };

    auto applyPath = [](const std::string& value, std::string& destination, bool cliOverride, const char* label) {
        if (!value.empty() && !cliOverride) {
            destination = value;
            std::cout << "Config library path (" << label << "): " << value << std::endl;
        }
    };

    YAML::Node libraryConfig;
    if (config["libraries"] && config["libraries"].IsMap()) {
        libraryConfig = config["libraries"];
    } else if (config["library_paths"] && config["library_paths"].IsMap()) {
        libraryConfig = config["library_paths"];
    }

    applyPath(
        readStringByKeys(libraryConfig, {"aptina", "aptina_library", "aptina_library_path"}),
        aptinaPluginLibraryPath,
        aptinaPluginLibraryPathCliOverride,
        "aptina plugin");
    applyPath(
        readStringByKeys(libraryConfig, {"pylon", "pylon_library", "pylon_library_path"}),
        pylonPluginLibraryPath,
        pylonPluginLibraryPathCliOverride,
        "pylon plugin");
    applyPath(
        readStringByKeys(libraryConfig, {"aptina_base", "aptina_base_library", "aptina_base_library_path"}),
        aptinaBaseLibraryPath,
        aptinaBaseLibraryPathCliOverride,
        "aptina base");
    applyPath(
        readStringByKeys(libraryConfig, {"pylon_base", "pylon_base_library", "pylon_base_library_path"}),
        pylonBaseLibraryPath,
        pylonBaseLibraryPathCliOverride,
        "pylon base");

    applyPath(
        readStringByKeys(config, {"aptina_library_path", "aptina_library"}),
        aptinaPluginLibraryPath,
        aptinaPluginLibraryPathCliOverride,
        "aptina plugin");
    applyPath(
        readStringByKeys(config, {"pylon_library_path", "pylon_library"}),
        pylonPluginLibraryPath,
        pylonPluginLibraryPathCliOverride,
        "pylon plugin");
    applyPath(
        readStringByKeys(config, {"aptina_base_library_path", "aptina_base_library"}),
        aptinaBaseLibraryPath,
        aptinaBaseLibraryPathCliOverride,
        "aptina base");
    applyPath(
        readStringByKeys(config, {"pylon_base_library_path", "pylon_base_library"}),
        pylonBaseLibraryPath,
        pylonBaseLibraryPathCliOverride,
        "pylon base");

    for (YAML::const_iterator it = config.begin(); it != config.end(); ++it) {
        if (!it->second.IsMap())
            continue;
        if (!it->second["type"])
            continue;
            
        // Create camera from file-defined details if the node is a camera node.
        if (it->second["type"].as<std::string>().compare("camera") == 0) {
            cameras[it->first.as<std::string>()] = std::make_unique<vidre::RuntimeInstance>();
            
            cameras[it->first.as<std::string>()]->setCamera(LoadCamera(it->second));
            
            auto storage_config = it->second["storage"];
            auto output_file = storage_config["output_file"].as<std::string>();
            auto header_format = storage_config["header"].as<std::string>();
            auto bits_per_pixel = storage_config["bits_per_pixel"].as<uint64_t>();
            auto packed = storage_config["packed"].as<uint8_t>();

            cameras[it->first.as<std::string>()]->createStorage(output_file, header_format, bits_per_pixel, packed);  
        }
    }

    return cameras;
}

int main(int argc, const char *argv[]) {
    // TODO: Separate Record System option/button
    // TODO: automatically detect master/triggered cam
    // TODO: first start all triggered cams, first finish master cam 
    std::cout << std::filesystem::current_path() << std::endl;
    // Load dynamic libraries for camera APIs before using any camera
    // const char* aptinaLibPath = "C:/Aptina Imaging/lib/apbase.lib"; //

    

    //const char* pylonLibPath = "libpylon.so"; // 
    // if (!load_aptina_library(aptinaLibPath)) {
    //    std::cerr << "Failed to load Aptina dynamic library: " << aptinaLibPath << std::endl;
    //     return -1;
    // }
    // else{
    //     std::cout << "LOADED APTINA\n";
    // }
    //if (!load_pylon_library(pylonLibPath)) {
    //    std::cerr << "Failed to load Pylon dynamic library: " << pylonLibPath << std::endl;
    //}

    



    // Parse arguments and set relevant variables.
    po::options_description desc("Allowed options");
    desc.add_options()
        ("help", "Print this help message")
        ("config", po::value<std::string>(), "File path to system configuration file")
        ("aptina-library", po::value<std::string>(), "Path to vidre Aptina camera plugin library")
        ("pylon-library", po::value<std::string>(), "Path to vidre Pylon camera plugin library")
        ("aptina-base-library", po::value<std::string>(), "Path to Aptina base runtime library")
        ("pylon-base-library", po::value<std::string>(), "Path to Pylon base runtime library");

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    if (vm.count("help")) {
        std::cout << desc << "\n";
        return 0;
    }
    if (vm.count("config")) {
        systemConfigFilepath = vm["config"].as<std::string>();
    }
    if (vm.count("aptina-library")) {
        aptinaPluginLibraryPath = vm["aptina-library"].as<std::string>();
        aptinaPluginLibraryPathCliOverride = true;
    }
    if (vm.count("pylon-library")) {
        pylonPluginLibraryPath = vm["pylon-library"].as<std::string>();
        pylonPluginLibraryPathCliOverride = true;
    }
    if (vm.count("aptina-base-library")) {
        aptinaBaseLibraryPath = vm["aptina-base-library"].as<std::string>();
        aptinaBaseLibraryPathCliOverride = true;
    }
    if (vm.count("pylon-base-library")) {
        pylonBaseLibraryPath = vm["pylon-base-library"].as<std::string>();
        pylonBaseLibraryPathCliOverride = true;
    }

    // Create system from YAML and check constructed cameras.
    std::unordered_map<std::string, std::unique_ptr<vidre::RuntimeInstance>> cameras = LoadSystemSetup(systemConfigFilepath);
    if (cameras.size() == 0) {
        std::cout << "Error in loading system config file." << std::endl;
        return 1;
    }
    for (auto it = cameras.begin(); it != cameras.end();) {
        std::cout << it->first << " was " << (it->second->checkCamera() ? "" : "not ") << "constructed.\n";
        if (!it->second)
            it = cameras.erase(it);
        else
            ++it;
    }
    std::cout.flush();



    // Simple Mock CLI
    // Mainly for testing out multiple cameras.
    std::string input;
    bool running = true;
    while (running) {
        std::cout << "[vidre] ";
        std::getline(std::cin, input);

        std::vector<std::string> tokens = split(input.c_str());
        if (tokens[0].compare("exit") == 0) {
            running = false;
            std::cout << "Exiting" << std::endl;
            continue;
        }
        else if (tokens[0].compare("list") == 0) {
            if (cameras.size() == 0) {
                std::cout << "No cameras loaded" << std::endl;
                continue;
            }
            for (const auto &it : cameras)
                std::cout << "- " << it.first << std::endl;
            continue;
        }

        if (tokens.size() < 2) {
            std::cout << "Insufficient arguments" << std::endl;
            continue;
        }
        
        // start all / stop all
        if (tokens[0] == "start" && tokens[1] == "all") {
            for (auto& cam: cameras) {
                if (!cam.second->isRunning()) {
                    cam.second->run();
                    std::cout << "Starting " << cam.first << std::endl;
                }
                else {
                    std::cout << cam.first << " already running" << std::endl;
                }
            }
            continue;
        }

        if (tokens[0] == "stop" && tokens[1] == "all") {
            for (auto& cam: cameras) {
                if (cam.second->isRunning()) {
                    std::cout << "Stopping " << cam.first << std::endl;
                    cam.second->stop();
                    std::cout << "Stopped " << cam.first << std::endl;
                }
                else {
                    std::cout << cam.first << " not running" << std::endl;
                }
            }
            continue;
        }

        auto match = cameras.find(tokens[1]);
        if (match == cameras.end()) {
            std::cout << tokens[1] << " not found" << std::endl;
            continue;
        }

        if (tokens[0].compare("start") == 0) {
            if (!match->second->isRunning()) {
                match->second->run();
                std::cout << "Starting " << match->first << std::endl;
            }
            else
                std::cout << match->first << " already running" << std::endl;
        }
        else if (tokens[0].compare("stop") == 0) {
            if (match->second->isRunning()) {
                std::cout << "Stopping " << match->first << std::endl;
                match->second->stop();
                std::cout << "Stopped " << match->first << std::endl;
            }
            else
                std::cout << match->first << " not running" << std::endl;
        }
    }

    for (const auto &it : cameras) {
        it.second->stop();
    }

    return 0;
}