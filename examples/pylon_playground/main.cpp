#include <iostream>

#include "vidre/runtime.h"


int main(int argc, char** argv) {
    std::string defaultConfigFilepath = std::string(argv[1]);

    auto runtime = std::make_unique<vidre::RuntimeInstance>(defaultConfigFilepath);

    runtime->run();
    
    std::cout << "Press Enter to stop.\n";
    while (std::cin.get() != '\n') {}

    runtime->stop();
    return 0;
}