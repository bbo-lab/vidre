#include <iostream>

#include <boost/asio.hpp>

#include "vidre/pylon_camera.hpp"

int main() {
    std::unique_ptr<vidre::ICamera> camera = std::make_unique<vidre::PylonCamera>(); // Or some factory class.
    // Eventually add Aptina Camera support.
    // std::unique_ptr<vidre::ICamera> camera = std::make_unique<vidre::AptinaCamera>();

    // Eventually there might be a way to generalize all GenICam (or USB3Vision) cameras under one class using GenAPI.
    // std::unique_ptr<vidre::ICamera> camera = std::make_unique<vidre::USB3VisionCamera>();

    camera->Init();

    camera->Arm();
    camera->Start();

    std::cout << "Press Enter to stop.\n";
    while(std::cin.get() != '\n') {};

    camera->Stop();
    camera->Disarm();

    return 0;
}