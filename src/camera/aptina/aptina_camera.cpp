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
            void* current_ptr = _buffer.CurrentBuffer(_buffer_num);

            ap_GrabFrame(_handle, static_cast<uint8_t*>(current_ptr), _buffer_size);
            
            auto computerTimeStamp = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now() - vidre::StartTime).count();

            
            SharedFrameHeader *result = (SharedFrameHeader *)(current_ptr);
            
            

            // TODO: 
            // auto fid = ap_GetFrameData(_handle, "frameNumber");
            // std::cout << fid << std::endl;
            // result->frameID = fid;

            result->frameID = retrievedFramesNumber;
            result->timeStamp = ap_GetFrameData(_handle, "frameTime");
            result->computerTimeStamp = computerTimeStamp;

            retrievedFramesNumber++;
        }
        std::cout << "RETRIEVED : " << retrievedFramesNumber << std::endl;
    }

    void AptinaCamera::Init() {
        _handle = ap_Create(0);
    }

    bool AptinaCamera::Init(std::string serial) {
        _serial = serial;
        std::cout << "H1" << std::endl;
        _handle = ap_Create(0);
        std::cout << "H2" << std::endl;
        return _handle != nullptr;
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
        return _buffer_size;
    }


    double AptinaCamera::GetFrameRate() {
        return 0;
    }

    std::string AptinaCamera::GetSerialNumber()
    {
        return _serial;
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
        std::cout <<"L1"<< std::endl;
        int result = ap_LoadIniPreset(_handle, filepath.c_str(), "EyeCAM");

        if (result != AP_INI_SUCCESS) {
            return false;
        }

        ap_GetImageFormat(_handle, &_w, &_h, nullptr, 0);

        _buffer_size = ap_GrabFrame(_handle, nullptr, 0);
        std::cout <<"L2"<< std::endl;
    }

    bool AptinaCamera::SaveConfiguration(std::string filepath) {
        return true;
    }

    void AptinaCamera::SetSharedMemory(boost::interprocess::managed_shared_memory* sharedMemory, std::vector<void *>* ptrs, std::string memory_name)
    {
        std::cout << "H3" << std::endl;
        _buffer.SetSharedMemory(sharedMemory, ptrs, memory_name);
        std::cout << "H5" << std::endl;
        std:: cout << ptrs->size() << std::endl;

        std::cout << _buffer_size << std::endl;

        for (int i = 0; i < _buffer_num; ++i) {
            _buffer.AllocateBuffer(_buffer_size);
        }
        std::cout << "H4" << std::endl;
    }
}  // namespace vidre
