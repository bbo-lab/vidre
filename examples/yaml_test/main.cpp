#include <iostream>
#include <string>
#include <unordered_map>

#include <boost/program_options.hpp>

#include "../src/utils/config_loader/config_loader.hpp"
#include "../src/core/runtime/runtime.hpp"
#include "utils/dynamic_loader/dynamic_library_loader.hpp"
// #include "../src/camera/pylon/pylon_api_loader.hpp"



namespace po = boost::program_options;

// std::string systemConfigFilepath = "C:/Not/A/Real/Path.yaml";
// std::string systemConfigFilepath = "C:/Projects/vidre/config/setupExample.yaml";
std::string systemConfigFilepath = "C:/Users/Almukhamedov_la/Desktop/work/testing2/vidre/config/setupExample.yaml";

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

int main(int argc, const char *argv[]) {
    // Load dynamic libraries for camera APIs before using any camera
    // const char* aptinaLibPath = "C:/Aptina Imaging/lib/apbase.lib"; //
    std::string aptinaLibPath= "C:/Aptina Imaging/bin/apbase.dll"; //

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
    desc.add_options()("help", "Print this help message")("config", po::value<std::string>(),
                                                          "File path to system configuration file");

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

    // Create system from YAML and check constructed cameras.
    std::unordered_map<std::string, std::unique_ptr<vidre::RuntimeInstance>> cameras = vidre::LoadSystemSetup(systemConfigFilepath);
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