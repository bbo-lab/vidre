#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

#include <boost/program_options.hpp>

#include <core/runtime/runtime.hpp>
#include <utils/config_loader/config_loader.hpp>

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

int main(int argc, const char *argv[]) {
    // TODO: Separate Record System option/button
    // TODO: automatically detect master/triggered cam
    // TODO: first start all triggered cams, first finish master cam 
    std::cout << std::filesystem::current_path() << std::endl;

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
    vidre::LoaderOptions loaderOptions;
    loaderOptions.aptinaLibraryPath = aptinaPluginLibraryPath;
    loaderOptions.pylonLibraryPath = pylonPluginLibraryPath;
    loaderOptions.aptinaBaseLibraryPath = aptinaBaseLibraryPath;
    loaderOptions.pylonBaseLibraryPath = pylonBaseLibraryPath;
    loaderOptions.aptinaLibraryPathCliOverride = aptinaPluginLibraryPathCliOverride;
    loaderOptions.pylonLibraryPathCliOverride = pylonPluginLibraryPathCliOverride;
    loaderOptions.aptinaBaseLibraryPathCliOverride = aptinaBaseLibraryPathCliOverride;
    loaderOptions.pylonBaseLibraryPathCliOverride = pylonPluginLibraryPathCliOverride;

    std::unordered_map<std::string, std::unique_ptr<vidre::RuntimeInstance>> cameras = vidre::LoadSystemSetup(
        systemConfigFilepath,
        loaderOptions,
        [](const std::string& message) { std::cout << message << std::endl; });
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
