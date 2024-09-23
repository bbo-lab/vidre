#include <vidre/module.hh>

#include <iostream>

using namespace vidre;

int main(int, const char **) {
    module::shared my;

    auto status = my.open(CMAKE_BINARY_DIR "/lib/" VIDRE_SHARED_MODULE(mymodule));

    std::cout << "status " << status.message() << std::endl;

    return 0;
}