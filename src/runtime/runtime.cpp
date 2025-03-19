#include "vidre/runtime.hpp"
#include "vidre/pylon_camera.hpp"
#include "vidre/shared_frame_header.hpp"
#include "vidre/storage.hpp"
#include <string>


static void presentFrame(void *dataBuffer, const vidre::Config &config) {
    Pylon::CPylonImage image;
    image.AttachUserBuffer(dataBuffer, config.sourceWidth * config.sourceHeight, config.sourcePixelType, config.sourceWidth,
                           config.sourceHeight, config.sourcePaddingX);
    Pylon::DisplayImage(1, image);
}

namespace vidre
{
    RuntimeInstance::~RuntimeInstance() {
        // Unsure where this last (and to be closed) PylonInit comes from.
        Pylon::PylonTerminate();
    }

    RuntimeInstance::RuntimeInstance(std::string configuration) {
        // switch (configuration.cameraVendor)  {
        //     // cameravendor is enum
        //     case CameraVendor::Pylon:
        //         _camera = std::make_unique<vidre::PylonCamera>();
        //     case CameraVendor::Aptina:
        //         _camera = std::make_unique<vidre::AptinaCamera>();
        //     default:
        //         _camera = nullptr;
        // }
        _camera = std::make_unique<vidre::PylonCamera>();


        // Initialize and start the camera
        _camera->Init();

        ICameraConfig *cameraParamConfiguration = _camera->GetCameraConfig();

        if (!cameraParamConfiguration->LoadConfiguration(configuration.c_str())) {
            std::cout << "Could not load configuration file \"" << configuration << "\" \n";
        }


        
        vidre::Config cameraConfig{};
        cameraConfig.sourcePixelType = Pylon::EPixelType::PixelType_Mono8;
        cameraConfig.sourcePaddingX = cameraParamConfiguration->GetFrameOffsetX();
        cameraConfig.sourceWidth = cameraParamConfiguration->GetFrameWidth();
        cameraConfig.sourceHeight = cameraParamConfiguration->GetFrameHeight();
        cameraConfig.sourceMaxNumBuffer = cameraParamConfiguration->GetMaxNumBuffer();
        cameraConfig.sourceFrameRate =  cameraParamConfiguration->GetFrameRate();
        cameraConfig.sourceSerialNumber = cameraParamConfiguration->GetSerialNumber();

        _storage = std::make_unique<vidre::Storage>(cameraConfig);

        _camera->SetSharedMemory(_storage->getSharedMemory(), _storage->getConfig().sourceSerialNumber);
    }

    void RuntimeInstance::setCamera(std::unique_ptr<ICamera> camera) {
        if (_camera != nullptr) {
            std::cout << "Camera is set already \n";
            return;
        }

        if (camera == nullptr) {
            std::cout << "Recieved nullptr camera \n";
            return;
        }

        _camera = std::move(camera);
    }

    void RuntimeInstance::createStorage(std::string outputFileName) {
        if (_storage != nullptr) {
            std::cout << "Storage is created already \n";
            return;
        }

        if (_camera == nullptr) {
            std::cout << "Camera is not set \n";
            return;
        }
        ICameraConfig *cameraParamConfiguration = _camera->GetCameraConfig();
        
        vidre::Config cameraConfig{};
        cameraConfig.sourcePixelType = Pylon::EPixelType::PixelType_Mono8;
        cameraConfig.sourcePaddingX = cameraParamConfiguration->GetFrameOffsetX();
        cameraConfig.sourceWidth = cameraParamConfiguration->GetFrameWidth();
        cameraConfig.sourceHeight = cameraParamConfiguration->GetFrameHeight();
        cameraConfig.sourceMaxNumBuffer = cameraParamConfiguration->GetMaxNumBuffer();
        cameraConfig.sourceFrameRate =  cameraParamConfiguration->GetFrameRate();
        cameraConfig.sourceSerialNumber = cameraParamConfiguration->GetSerialNumber();
        _storage = std::make_unique<vidre::Storage>(cameraConfig, outputFileName);

        _camera->SetSharedMemory(_storage->getSharedMemory(), _storage->getConfig().sourceSerialNumber);
    }

    void RuntimeInstance::retrieveFrames() {
        const vidre::SharedFrameHeader* previousResult = nullptr;
        
        // Wait for Pylon's CInstantCamera to start running.
        while (!_camera->IsRunning()) {
            std::this_thread::yield();
        }

        // As long as the camera is acquiring frames, (display the new frame) and save it to disk.
        while (_camera->IsRunning()) {
            // Retrieve frame
            const auto* result = _camera->RetrieveLatestFrame();
            
            if (result == previousResult) {
                std::this_thread::yield();
                continue;
            }
            // update ccv_header
            _storage->updateCCVHeader();

            // create_tail
            vidre::CCVFrameTail tail;
            tail.frameIndex = result->frameID; // + result->numSkippedBefore? TODO: think about skipped logic
            // tail.skippedFrames = numSkippedBefore;
            tail.timeStamp = result->timeStamp;
            tail.timeStamp /= 1e9;

            _storage->appendCCVFrame((const void *)(result + 1), tail);
         
         
            previousResult = result;

            if (_saveTIFF) {
                _storage->save((const void *)(result + 1), result->frameID);
            }
            
            if (_presentFrame) {
                presentFrame((void *)(result + 1), _storage->getConfig());
            }
        }
    }

    void RuntimeInstance::run() {
        if (_camera == nullptr) {
            return;
        }

        if (_storage == nullptr) {
            return;
        }

        _camera->Arm();
        _camera->Start();
        
        _thread = std::thread(&RuntimeInstance::retrieveFrames, this);
    }

    void RuntimeInstance::stop() {
        // todo: maybe clear storage
        if (_camera == nullptr) {
            return;
        }

        _camera->Stop();
        
        if (_thread.joinable()) {
            _thread.join();
        }
        
        _camera->Disarm();
    }

    bool RuntimeInstance::checkCamera() {
        return _camera != nullptr;
    }

}