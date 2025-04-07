#include "vidre/runtime.hpp"
#include "vidre/pylon_camera.hpp"
#include "vidre/shared_frame_header.hpp"
#include "vidre/storage.hpp"
#include <string>


static void presentFrame(void *dataBuffer, const vidre::Config &config) {
    Pylon::CPylonImage image;
    image.AttachUserBuffer(dataBuffer, config.width * config.height, config.pixelType, config.width,
                           config.height, config.paddingX);
    Pylon::DisplayImage(1, image);
}

namespace vidre
{
    RuntimeInstance::~RuntimeInstance() {
        // Unsure where this last (and to be closed) PylonInit comes from.
        Pylon::PylonTerminate();
    }

    RuntimeInstance::RuntimeInstance(std::string configuration, std::string header_format) {
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

        if (!_camera->LoadConfiguration(configuration.c_str())) {
            std::cout << "Could not load configuration file \"" << configuration << "\" \n";
        }


        
        vidre::Config cameraConfig{};
        cameraConfig.pixelType = Pylon::EPixelType::PixelType_Mono8;
        cameraConfig.paddingX = _camera->GetFrameOffsetX();
        cameraConfig.width = _camera->GetFrameWidth();
        cameraConfig.height = _camera->GetFrameHeight();
        cameraConfig.maxNumBuffer = _camera->GetMaxNumBuffer();
        cameraConfig.frameRate =  _camera->GetFrameRate();
        cameraConfig.serialNumber = _camera->GetSerialNumber();

        _storage = std::make_unique<vidre::Storage>(cameraConfig, header_format);

        _camera->SetSharedMemory(_storage->getM(), _storage->getSharedMemory(), _storage->getConfig().serialNumber);
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

    void RuntimeInstance::createStorage(std::string output_file_name, std::string header_format) {
        if (_storage != nullptr) {
            std::cout << "Storage is created already \n";
            return;
        }

        if (_camera == nullptr) {
            std::cout << "Camera is not set \n";
            return;
        }
        
        vidre::Config cameraConfig{};
        cameraConfig.pixelType = Pylon::EPixelType::PixelType_Mono8;
        cameraConfig.paddingX = _camera->GetFrameOffsetX();
        cameraConfig.width = _camera->GetFrameWidth();
        cameraConfig.height = _camera->GetFrameHeight();
        cameraConfig.maxNumBuffer = _camera->GetMaxNumBuffer();
        cameraConfig.frameRate = _camera->GetFrameRate();
        cameraConfig.serialNumber = _camera->GetSerialNumber();

        _storage = std::make_unique<vidre::Storage>(cameraConfig, header_format, output_file_name);

        _camera->SetSharedMemory(_storage->getM(), _storage->getSharedMemory(), _storage->getConfig().serialNumber);
    }

    void RuntimeInstance::retrieveFrames() {
        while (!_camera->IsRunning()) {
            std::this_thread::yield();
        }

        int prev_frame_id = 0;
        while (_camera->IsRunning()) {
            const auto* result = _storage->retrieveFrame(_currentFrame);
           
            if (result == nullptr || result->frameID == 0 || prev_frame_id >= result->frameID) {
                continue;
            }
           
            if (result->frameID > prev_frame_id + 1) {
                std::cout << "WARNING: RETRIEVED FRAME BIGGER THAN BUFFER + 1\n";
            }
           
            prev_frame_id = result->frameID;
            
            _storage->appendFrame(static_cast<const void *>(result + 1));
            _storage->appendTail(result->frameID, result->timeStamp);

            if (_saveTIFF) {
                _storage->save(static_cast<const void *>(result + 1), result->frameID);
            }
            
            _currentFrame++;
        }


        // todo: add waiting for camera really fully stops, i.e runtime need control over cam thread
        // until then -1 saved frame in most cases
        for (int i = 0; i <= _storage->getConfig().maxNumBuffer; ++i) 
        {
            const auto* result = _storage->retrieveFrame(i);
            
            if (result == nullptr || result->frameID == 0 || prev_frame_id >= result->frameID) {
                continue;
            }

            if (result->frameID > prev_frame_id + 1) {
                std::cout << "WARNING: RETRIEVED FRAME BIGGER THAN BUFFER + 1\n";
            }

            prev_frame_id = result->frameID;

            _storage->appendFrame(static_cast<const void *>(result + 1));
            _storage->appendTail(result->frameID, result->timeStamp);

            if (_saveTIFF) {
                _storage->save(static_cast<const void *>(result + 1), result->frameID);
            }
            
            _currentFrame++;
        }

        _storage->updateHeader(_currentFrame);
        // std::cout << "SAVED : " << _currentFrame << std::endl;
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