#include "vidre/aptina_camera.hpp"

#include <iostream>

namespace vidre {
    AptinaCamera::AptinaCamera() : running(false), armed(false) , _handle(nullptr) {}
    AptinaCamera::~AptinaCamera() {
        ap_Finalize();
    }

    void AptinaCamera::run() {
        running.store(true);
        
        while (running.load()) {
            auto [buffer, header] = _buffer.CurrentBuffer(_buffer_num);

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
        _handle = ap_Create(0);
        if (_handle == nullptr) {
            std::cout << "Failed to Init aptina\n";
            
        }
    }

    bool AptinaCamera::Init(std::string serial) {
        _serial = serial;

        _handle = ap_Create(0);
        if (_handle == nullptr) {
            std::cout << "Failed to Init aptina\n";
            return false;
        }
        ap_SetState(_handle, "Sensor Reset", 1);
        #ifdef _WIN32
            Sleep(10);
        #else
            sleep(1);
        #endif
            ap_SetState(_handle, "Sensor Reset", 0);
        #ifdef _WIN32
            Sleep(10);
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

    
    void AptinaCamera::SetTriggerIn(unsigned int line) {
    }
    void AptinaCamera::SetTriggerOut(unsigned int line) {
    }

    bool AptinaCamera::LoadConfiguration(std::string filepath) {
        // get buffer data
        // int result = ap_LoadIniPreset(_handle, filepath.c_str(), "Reset");
        // int result = ap_LoadIniPreset(_handle, filepath.c_str(), "Reset");

        // if (result != AP_INI_SUCCESS) {
        //     std::cout << "Failed to load config for  aptina\n";
        //     return false;
        // }

        // Sleep(500);

        // ap_CheckSensorState(_handle, 0);

        // ap_GetImageFormat(_handle, &_w, &_h, nullptr, 0);
        // std::cout << _w << " " << _h << std::endl;

        // _buffer_size_min = ap_GrabFrame(_handle, nullptr, 0);
        // auto success = ap_GetLastError();
        // if (success != AP_CAMERA_SUCCESS) {
        //     std::cout << "APTINA FAILED TO GRAB 1 \n";
        //     return false;
        // }
        
        // char image_type[64];
        // ap_GetImageFormat (_handle, &_w, &_h, image_type,
        //     sizeof (image_type));


        // int bpp = 0;
        // if (std::string(image_type) == "BAYER-10") {
        //     int bpp = 10;
        // } else if (std::string(image_type) == "BAYER-8") {
        //     int bpp = 8;
        // }
        _buffer_size_save = (int)ceil(752 * 480 * 10 / 8.0);
        _buffer_size_min = _buffer_size_save * 2;

        // _buffer_size_save =  _buffer_size_min; // actual buffer size to save (can also be acquired through BAYER value and /8 etc.)

        // // actually load triggered config
        // ap_Destroy(_handle);
        // ap_Finalize();

        // _handle = ap_Create(0);
        // if (_handle == nullptr) {
        //     std::cout << "Failed to re-create aptina\n";
        //     return false;
        // }

        // result = ap_LoadIniPreset(_handle, filepath.c_str(), "EyeCAM Triggered");
        // if (result != AP_INI_SUCCESS) {
        //     std::cout << "Failed to load trigger config for aptina\n";
        //     return false;
        // }

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
