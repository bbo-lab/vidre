#include <iostream>

#include <string>
#include <unordered_map>

#include <boost/program_options.hpp>

#include "vidre/config_loader.hpp"
#include "vidre/camera.hpp"

namespace po = boost::program_options;

std::string systemConfigFilepath = "C:/Blip/Blap/Bloop";  //"C:/Projects/vidre/config/setupExample.yaml";

int main(int argc, const char *argv[]) {
    // Parse arguments
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

    std::unordered_map<std::string, std::unique_ptr<vidre::ICamera>> cameras = vidre::LoadSystemSetup(systemConfigFilepath);
    if (cameras.size() == 0) {
        std::cout << "Error in loading system config file." << std::endl;
        return 1;
    }
    for (auto it = cameras.begin(); it != cameras.end(); ++it) {
        std::cout << it->first << " was " << (it->second ? "" : "not ") << "constructed.\n";
    }
    std::cout.flush();

    return 0;
}