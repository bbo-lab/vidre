// #include "aptina_api_loader.hpp"
#include "aptina_camera.hpp"

#include <iostream>


// TODO: check whether if(ap_F) is needed

namespace vidre {
    AptinaCamera::AptinaCamera() : running(false), armed(false) , _handle(nullptr) {}
    AptinaCamera::~AptinaCamera() {
        if (ap_Finalize) ap_Finalize();
    }

    void AptinaCamera::run() {
        running.store(true);
        while (running.load()) {
            auto [buffer, header] = _buffer.CurrentBuffer(_buffer_num);
            if (!ap_GrabFrame || !ap_GetLastError) {
                std::cerr << "Aptina API not loaded!\n";
                return;
            }
            ap_GrabFrame(_handle, buffer, _buffer_size_min);
            auto computerTimeStamp = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now() - vidre::StartTime
            ).count();
            ++retrievedFramesNumber;
            auto success = ap_GetLastError();
            if (success != AP_CAMERA_SUCCESS) {
                std::cout << "APTINA FAILED TO GRAB\n";
                return;
            }

            
            // TODO: 
            // auto fid = ap_GetFrameData(_handle, "frameNumber");
            // std::cout << fid << std::endl;
            // result->frameID = fid;

            header->frameID = retrievedFramesNumber;
            // header->timeStamp = ap_GetFrameData(_handle, "frameTime");
            header->timeStamp = computerTimeStamp;
            header->computerTimeStamp = computerTimeStamp;
        }
        std::cout << "RETRIEVED : " << retrievedFramesNumber << std::endl;
    }

    void AptinaCamera::Init() {
        if (!ap_Create) {
            std::cerr << "Aptina API not loaded!\n";
            return;
        }
        _handle = ap_Create(0);
        if (_handle == nullptr) {
            std::cout << "Failed to Init aptina\n";
        }
    }

    bool AptinaCamera::Init(std::string serial) {
        _serial = serial;
        if (!ap_Create || !ap_SetState) {
            std::cerr << "Aptina API not loaded!\n";
            return false;
        }
        _handle = ap_Create(0);
        if (_handle == nullptr) {
            std::cout << "Failed to Init aptina\n";
            return false;
        }
        ap_SetState(_handle, "Sensor Reset", 1);
        #ifdef _WIN32
            Sleep(1000);
        #else
            sleep(1);
        #endif
            ap_SetState(_handle, "Sensor Reset", 0);
        #ifdef _WIN32
            Sleep(1000);
        #else
            sleep(1);
        #endif
        return true;
    }

    void AptinaCamera::Arm() {
    }
    void AptinaCamera::Disarm() {
    }
    void AptinaCamera::Start() {
        grabber = std::thread(&AptinaCamera::run, this);
    }
    void AptinaCamera::Stop() {
        running.store(false);
        if (grabber.joinable())
            grabber.join();
    }

    const SharedFrameHeader* AptinaCamera::RetrieveLatestFrame() {
        const SharedFrameHeader* a;
        return a;
    }
    bool AptinaCamera::IsArmed() {
        return armed.load();
    }
    bool AptinaCamera::IsRunning() {
        return grabber.joinable();
    }

    unsigned int AptinaCamera::GetFrameWidth() {
       return _w;
    }

    unsigned int AptinaCamera::GetFrameHeight() {
        return _h;
    }

    unsigned int AptinaCamera::GetFrameOffsetX() {
        return 0;
    }

    unsigned int AptinaCamera::GetFrameOffsetY() {
        return 0;
    }

    unsigned int AptinaCamera::GetMaxNumBuffer() {
        return _buffer_num;
    }
    unsigned int AptinaCamera::GetFrameBytes() {
        return _buffer_size_save;
    }
    unsigned int AptinaCamera::GetBufferSize() {
        return _buffer_size_min;
    }



    double AptinaCamera::GetFrameRate() {
        return 50;
    }

    std::string AptinaCamera::GetSerialNumber() {
        return _serial;
    }

    std::string AptinaCamera::GetVendorName() {
        return "aptina";
    }

    void AptinaCamera::SetFrameWidth(unsigned int newFrameWidth) {
       
    }

    void AptinaCamera::SetFrameHeight(unsigned int newFrameHeight) {
    }

    void AptinaCamera::SetFrameOffsetX(unsigned int newFrameOffsetX) {
    }

    void AptinaCamera::SetFrameOffsetY(unsigned int newFrameOffsetY) {
    }

    void AptinaCamera::SetFrameRate(double newFrameRate) {
    }
    
    void AptinaCamera::SetExposure(unsigned int newExposure) {
        int result = ap_SetSensorRegister(_handle, "xstdat", nullptr, newExposure, nullptr);
        if (result != AP_INI_SUCCESS) {
            std::cout << "Failed to set sensor register\n";
        }
    }

    void AptinaCamera::SetGain(unsigned int newGain) { // TODO
        int result = ap_SetSensorRegister(_handle, "xstdat", nullptr, newGain, nullptr); // ANALOG_GAIN/global
        if (result != AP_INI_SUCCESS) {
            std::cout << "Failed to set sensor register\n";
        }
    }
    
    void AptinaCamera::SetTriggerIn(unsigned int line) {
        if (!ap_Destroy || !ap_Finalize || !ap_LoadIniPreset) {
            std::cerr << "Aptina API not loaded!\n";
            return;
        }
        ap_Destroy(_handle);
        ap_Finalize();
        Init(_serial);
        int result = ap_LoadIniPreset(_handle, _config_file.c_str(), "EyeCAM-Triggered");
        if (result != AP_INI_SUCCESS) {
            std::cout << "Failed to load trigger config for aptina\n";
            return;
        }
    }
    
    void AptinaCamera::SetTriggerOut(unsigned int line) {
    }

    bool AptinaCamera::LoadConfiguration(std::string filepath) {
        if (!ap_LoadIniPreset || !ap_CheckSensorState || !ap_GetImageFormat || !ap_GrabFrame || !ap_GetLastError) {
            std::cerr << "Aptina API not loaded!\n";
            return false;
        }
        int result = ap_LoadIniPreset(_handle, filepath.c_str(), "EyeCAM");
        if (result != AP_INI_SUCCESS) {
            std::cout << "Failed to load config for  aptina\n";
            return false;
        }
        Sleep(500);
        ap_CheckSensorState(_handle, 0);
        ap_GetImageFormat(_handle, &_w, &_h, nullptr, 0);
        _buffer_size_min = ap_GrabFrame(_handle, nullptr, 0);
        auto success = ap_GetLastError();
        if (success != AP_CAMERA_SUCCESS) {
            std::cout << "APTINA FAILED TO GRAB 1 \n";
            return false;
        }
        uint8_t* b = new uint8_t[_buffer_size_min * 2];
        for (auto cnt = 0; cnt < 100; ++cnt) {
            _buffer_size_save = ap_GrabFrame(_handle, b, 2 * _buffer_size_min);
            success = ap_GetLastError();
            if (success == AP_CAMERA_SUCCESS) {
                break;
            }
        }
        delete[] b;
        b = NULL;

        _config_file = filepath;

        return true;
    }

    bool AptinaCamera::SaveConfiguration(std::string filepath) {
        return true;
    }

    void AptinaCamera::SetSharedMemory(boost::interprocess::managed_shared_memory* sharedMemory, std::vector<void *>* ptrs, std::string memory_name)
    {
        _buffer.SetSharedMemory(sharedMemory, ptrs, memory_name);

        for (int i = 0; i < _buffer_num; ++i) {
            _buffer.AllocateBuffer(_buffer_size_min);
        }
    }
}  // namespace vidre


extern "C" {

VIDRE_CAMERA_API vidre::ICamera* create_aptina_camera() {
    return new vidre::AptinaCamera();
}

VIDRE_CAMERA_API void destroy_aptina_camera(vidre::ICamera* camera) {
    delete camera;
}
}