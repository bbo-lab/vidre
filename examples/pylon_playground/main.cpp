#include <iostream>

#include "vidre/pylon_camera.hpp"

int main() {
    vidre::PylonCamera camera;
    camera.Init();
    // Configure static camera parameters that are not changed during grabbing (e.g. resolution).
    // e.g. camera.setRaw(filepath_to_configuration);

    camera.Start();
    Pylon::WaitObject::Sleep(1000);
    camera.Stop();
    Pylon::WaitObject::Sleep(1000);
    camera.Start();
    Pylon::WaitObject::Sleep(1000);
    camera.Stop();
    return 0;
}