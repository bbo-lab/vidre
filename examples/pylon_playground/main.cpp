#include <iostream>

#include "pylon_camera.hpp"

int main() {
    PylonCamera camera;
    camera.init();
    std::cout << "Resolution: (" << camera.getImageWidth() << ", " << camera.getImageHeight() << ")" << std::endl;

    // Configure static camera parameters that are not changed during grabbing (e.g. resolution).
    // e.g. camera.setRaw(filepath_to_configuration);

    camera.arm();
    camera.start();
    Pylon::WaitObject::Sleep(1000);
    camera.stop();
    Pylon::WaitObject::Sleep(1000);
    camera.start();
    Pylon::WaitObject::Sleep(1000);
    camera.stop();
    camera.disarm();
    return 0;
}