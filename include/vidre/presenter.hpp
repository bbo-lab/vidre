#pragma once
#include <iostream>

// #include <pylon/PylonGUI.h>
#include "pylon_camera.hpp"
#include "vidre/shared_frame_header.hpp"
#include "storage.h"

namespace vidre {
    class Presenter {
    public:
        Presenter() {
            window.Create(0);
        }
        ~Presenter() {
            Stop();
            window.Close();
        }
        void SetCamera(ICamera *newCamera) {
            camera = newCamera;
        }
        ICamera *GetCamera() {
            return camera;
        }
        void Start() {
            presenting = true;
            thread = std::thread(&Presenter::presentationThread, this, camera);
            window.Show();
        }
        void Stop() {
            if (!presenting)
                return;
            window.Hide();
            presenting = false;
            thread.join();
        }
        bool IsRunning() {
            return presenting;
        }

    private:
        std::thread thread;
        std::atomic<bool> presenting;
        ICamera *camera;
        Pylon::CPylonImageWindow window;

        void presentFrame(void *dataBuffer, const vidre::Config &config) {
            Pylon::CPylonImage image;
            image.AttachUserBuffer(dataBuffer, config.sourceWidth * config.sourceHeight, config.sourcePixelType, config.sourceWidth,
                                   config.sourceHeight, config.sourcePaddingX);
            window.SetImage(image);
            window.Show();
        }

        void presentationThread(vidre::ICamera *camera) {
            // Setting up relevant camera data for saving
            vidre::Config cameraConfig{};
            cameraConfig.sourcePixelType = Pylon::EPixelType::PixelType_Mono8;
            cameraConfig.sourcePaddingX = camera->GetFrameOffsetX();
            cameraConfig.sourceWidth = camera->GetFrameWidth();
            cameraConfig.sourceHeight = camera->GetFrameHeight();

            const vidre::SharedFrameHeader *result = nullptr, *previousResult = nullptr;

            // Wait for Pylon's CInstantCamera to start running.
            while (!camera->IsRunning()) {
                std::this_thread::yield();
            }

            // As long as the camera is acquiring frames, (display the new frame) and save it to disk.
            while (presenting) {
                // Retrieve frame
                result = camera->RetrieveLatestFrame();  // Unsafe: Pointer to const is not actually constant
                if (result == previousResult) {
                    std::this_thread::yield();
                    continue;
                }

                // Present frame
#ifdef PYLON_WIN_BUILD
                presentFrame((void *)(result + 1), cameraConfig);
#endif
                previousResult = result;
            }
        }
    };
}  // namespace vidre