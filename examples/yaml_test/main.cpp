#include <iostream>
#include <string>
#include <unordered_map>

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
        std::cout << "> " << it->first << " was " << (it->second ? "" : "not ") << "constructed.\n";
    }
    std::cout.flush();

    // Arm and start all cameras
    for (auto it = cameras.begin(); it != cameras.end(); ++it) {
        if (!it->second)
            continue;
        it->second->Arm();
        it->second->Start();
    }

    // Create presenter that presents the output of camOverview (i.e. piping camOverview output to presenter input)
    // Run until Enter.
    {
        vidre::Presenter presenter(cameras["camOverview"].get());  // Starts thread on creation, ends on scoped deletion.

        std::cout << "> \tPress Enter to stop.";
        while (std::cin.get() != '\n') {
        }
    }

    // Disarm and stop all cameras
    for (auto it = cameras.begin(); it != cameras.end(); ++it) {
        if (!it->second)
            continue;
        it->second->Stop();
        it->second->Disarm();
    }

    return 0;
}