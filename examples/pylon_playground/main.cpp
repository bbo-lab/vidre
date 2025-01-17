#include <iostream>

#include "vidre/runtime.h"


int main(int argc, char** argv) {
    std::string defaultConfigFilepath = std::string(argv[1]);

    vidre::RuntimeInstance runtime = std::make_unique<vidre::RuntimeInstance>(defaultConfigFilepath);

    int n_iter = 1;
    runtime->run(n_iter);

    return 0;
}