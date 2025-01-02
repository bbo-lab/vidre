#include <iostream>

#include "vidre/pylon_camera.hpp"

int main() {
    std::unique_ptr<vidre::ICamera> camera = std::make_unique<vidre::PylonCamera>(); // Or some factory class.
    // Eventually add Aptina Camera support. AND/OR
    // Eventually there might be a way to generalize all GenICam (or USB3Vision) cameras under one class using GenAPI.

    camera->Init();
    camera->Start();
    for (int i=0; i<=200; ++i) {
        SharedFrameHeader* result = camera->RetrieveLatestFrame();
        // Save
    }
    //std::cout << result->frame_id << " at time " << result->time_stamp / 1000000000 << std::endl;
    Pylon::WaitObject::Sleep(500);
    camera->Stop();
    Pylon::WaitObject::Sleep(1000);
    camera->Start();
    Pylon::WaitObject::Sleep(1000);
    camera->Stop();
    return 0;
}