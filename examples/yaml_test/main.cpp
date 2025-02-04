#include <iostream>
#include <string>
#include <unordered_map>
#include <chrono>

#include <boost/program_options.hpp>

#include "vidre/config_loader.hpp"
#include "vidre/camera.hpp"
#include "vidre/presenter.hpp"

namespace po = boost::program_options;

// std::string systemConfigFilepath = "C:/Not/A/Real/Path.yaml";
std::string systemConfigFilepath = "C:/Projects/vidre/config/setupExample.yaml";

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
    std::unordered_map<std::string, std::unique_ptr<vidre::ICamera>> cameras = vidre::LoadSystemSetup(systemConfigFilepath);
    if (cameras.size() == 0) {
        std::cout << "Error in loading system config file." << std::endl;
        return 1;
    }
    for (auto it = cameras.begin(); it != cameras.end(); ++it) {
        std::cout << it->first << " was " << (it->second ? "" : "not ") << "constructed.\n";
    }
    std::cout.flush();

    cameras["camOverview"]->Arm();
    cameras["camOverview"]->Start();
    {
        vidre::Presenter presenter(cameras["camOverview"].get());
        using namespace std::chrono_literals;
        std::this_thread::sleep_for(3s);
    }

    cameras["camOverview"]->Stop();
    cameras["camOverview"]->Disarm();

    return 0;
}