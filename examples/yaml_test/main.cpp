#include <iostream>
#include <string>
#include <unordered_map>

#include <boost/program_options.hpp>

#include "vidre/config_loader.hpp"
#include "vidre/camera.hpp"
#include "vidre/presenter.hpp"

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
        std::cout << it->first << " was " << (it->second->CheckCamera() ? "" : "not ") << "constructed.\n";
        if (!it->second)
            it = cameras.erase(it);
        else
            ++it;
    }
    std::cout.flush();


    // vidre::Presenter presenter;

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
            // presenter.Stop();
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
        auto match = cameras.find(tokens[1]);
        if (match == cameras.end()) {
            std::cout << tokens[1] << " not found" << std::endl;
            continue;
        }

        if (tokens[0].compare("start") == 0) {
            if (!match->second->IsRunning()) {
                match->second->run();
                std::cout << "Starting " << match->first << std::endl;
            }
            else
                std::cout << match->first << " already running" << std::endl;
        }
        else if (tokens[0].compare("stop") == 0) {
            if (match->second->IsRunning()) {
                match->second->stop();
                std::cout << "Stopping " << match->first << std::endl;
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