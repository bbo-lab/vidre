#include <core/runtime/runtime.hpp>
#include <core/runtime/shared_frame_header.hpp>


namespace vidre
{
    // RuntimeInstance::~RuntimeInstance() {
    // }

    // RuntimeInstance::RuntimeInstance(std::string configuration, std::string header_format) {
    //     _camera = std::make_unique<vidre::PylonCamera>();

    //     // Initialize and start the camera
    //     _camera->Init();

    //     if (!_camera->LoadConfiguration(configuration.c_str())) {
    //         std::cout << "Could not load configuration file \"" << configuration << "\" \n";
    //     }


        
    //     vidre::Config cameraConfig{};
    //     cameraConfig.pixelType = Pylon::EPixelType::PixelType_Mono8;
    //     cameraConfig.paddingX = _camera->GetFrameOffsetX();
    //     cameraConfig.width = _camera->GetFrameWidth();
    //     cameraConfig.height = _camera->GetFrameHeight();
    //     cameraConfig.maxNumBuffer = _camera->GetMaxNumBuffer();
    //     cameraConfig.frameRate =  _camera->GetFrameRate();
    //     cameraConfig.serialNumber = _camera->GetSerialNumber();
    //     cameraConfig.frameBytesActual = _camera->GetFrameBytes();
    //     cameraConfig.frameBytesBuffer = _camera->GetBufferSize();

    //     _storage = std::make_unique<vidre::Storage>(cameraConfig, header_format);

    //     _buffer = std::make_unique<vidre::Buffer>(cameraConfig);

    //     _camera->SetSharedMemory(_buffer->getSharedMemory(), _buffer->getSharedMemoryPtrs(), _storage->getConfig().serialNumber);
    // }

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

    void RuntimeInstance::createStorage(std::string output_file_name, std::string header_format, uint64_t bits_per_pixel, uint8_t packed) {
        if (_camera == nullptr) {
            std::cout << "Camera is not set \n";
            return;
        }

        // Store params for recreating on subsequent runs
        _storageOutputFileName = output_file_name;
        _storageHeaderFormat = header_format;
        _storageBitsPerPixel = bits_per_pixel;
        _storagePacked = packed;
        _storageConfigured = true;
        
        vidre::Config cameraConfig{};
        cameraConfig.paddingX = _camera->GetFrameOffsetX();
        cameraConfig.width = _camera->GetFrameWidth();
        cameraConfig.height = _camera->GetFrameHeight();
        cameraConfig.maxNumBuffer = _camera->GetMaxNumBuffer();
        cameraConfig.frameRate = _camera->GetFrameRate();
        cameraConfig.serialNumber = _camera->GetSerialNumber();
        cameraConfig.frameBytesActual = _camera->GetFrameBytes();
        cameraConfig.frameBytesBuffer = _camera->GetBufferSize();
        cameraConfig.cameraVendor = _camera->GetVendorName();
        cameraConfig.bitsPerPixel = bits_per_pixel;
        cameraConfig.packed = packed;

        _storage = std::make_unique<vidre::Storage>(cameraConfig, header_format, output_file_name);

        _buffer = std::make_unique<vidre::Buffer>(cameraConfig);

        _camera->SetSharedMemory(_buffer->getSharedMemory(), _buffer->getSharedMemoryPtrs(), _storage->getConfig().serialNumber);
    }


    void RuntimeInstance::retrieveFramesWithSave() {
        while (!_camera->IsRunning()) {
            std::this_thread::yield();
        }

        _recordingStatus = RecordingStatus::RecordingFine;
        uint32_t expectedFrameID = 0;
        bool firstFrame = true;

        while (_camera->IsRunning()) {
            const auto [result, header] = _buffer->pop();
            if (result == nullptr) {
                continue;
            }
            
            if (firstFrame) {
                expectedFrameID = header->frameID;
                firstFrame = false;
            } else if (header->frameID > expectedFrameID) {
                _recordingStatus = RecordingStatus::RecordingDroppedFrames;
            }
            expectedFrameID = header->frameID + 1;

            _latestFrameData = static_cast<uint8_t*>(result);
            _currentFrame++;
           
            _storage->appendFrame(static_cast<const void *>(result));
            _storage->appendTail(header->frameID, header->timeStamp, header->computerTimeStamp);

            if (_saveTIFF) {
                _storage->save(static_cast<const void *>(result), header->frameID);
            }
        }

        _storage->updateHeader(_buffer->getSavedFramesNumber());

        std::cout << "SAVED : " << _buffer->getSavedFramesNumber() << std::endl;
    }

    void RuntimeInstance::showFrames() { // showing is implemented in app, otherwise -> run + ccv_tools
        while (!_camera->IsRunning()) {
            std::this_thread::yield();
        }

        _recordingStatus = RecordingStatus::Streaming;

        while (_camera->IsRunning()) {
            const auto [result, header] = _buffer->pop();
            if (result == nullptr) {
                continue;
            }
            _currentFrame++;
            _latestFrameData = static_cast<uint8_t*>(result);
        }
    }

    void RuntimeInstance::run() {
        if (_camera == nullptr) {
            return;
        }

        // Recreate storage and buffer if they were cleaned up from a previous run
        if (!_storage || !_buffer) {
            recreateStorageAndBuffer();
        }

        if (_storage == nullptr || _buffer == nullptr) {
            return;
        }

        _camera->Arm();
        
        _storage->initiateHeader();
        
        _camera->Start();
        
        _thread = std::thread(&RuntimeInstance::retrieveFramesWithSave, this);
    }
    void RuntimeInstance::show() {
        if (_camera == nullptr) {
            return;
        }

        if (!_storage || !_buffer) {
            recreateStorageAndBuffer();
        }

        if (_storage == nullptr || _buffer == nullptr) {
            return;
        }

        _camera->Arm();
        
        _camera->Start();
        
        _thread = std::thread(&RuntimeInstance::showFrames, this);
    }

    void RuntimeInstance::stop() {
        if (_camera == nullptr) {
            return;
        }

        _camera->Stop();
        
        if (_thread.joinable()) {
            _thread.join();
        }
        
        _camera->Disarm();
        _recordingStatus = RecordingStatus::Stopped;

        resetForNextRun();
    }

    void RuntimeInstance::resetForNextRun() {
        _currentFrame = 0;
        _latestFrameData = nullptr;

        // Release old storage and buffer so they can be recreated on the next run
        _storage.reset();
        _buffer.reset();
    }

    void RuntimeInstance::recreateStorageAndBuffer() {
        if (!_storageConfigured || !_camera) {
            return;
        }

        // Recreate storage and buffer fresh for the new run
        createStorage(_storageOutputFileName, _storageHeaderFormat, _storageBitsPerPixel, _storagePacked);
    }

    bool RuntimeInstance::checkCamera() {
        return _camera != nullptr;
    }
    bool RuntimeInstance::isRunning() {
        return _camera && _camera->IsRunning();
    }

    RecordingStatus RuntimeInstance::getRecordingStatus() {
        return _recordingStatus;
    }

}