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
        return _buffer_size_save;
    }
    unsigned int AptinaCamera::GetBufferSize() {
        return _buffer_size_min;
    }



    double AptinaCamera::GetFrameRate() {
        return 50;
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

        // target frame size on disk in B:
        // 1. w * h * depth / 8 (depth depends on image_type, i.e Bpp / bayer-X values)
        // 2. create dummy buf and run grabframe again


        std::cout <<"L1"<< std::endl;
        int result = ap_LoadIniPreset(_handle, filepath.c_str(), "EyeCAM");

        if (result != AP_INI_SUCCESS) {
            return false;
        }

        ap_GetImageFormat(_handle, &_w, &_h, nullptr, 0);

        _buffer_size_min = ap_GrabFrame(_handle, nullptr, 0);
        std::cout << "TO ALLOCATE : " << _buffer_size_min << std::endl;

        uint8_t* b = new uint8_t[_buffer_size_min];
        _buffer_size_save = ap_GrabFrame(_handle, b, _buffer_size_min);
        delete[] b;

        std::cout << "TO save : " << _buffer_size_save << std::endl;


        // BAYER STAFF
        // char image_type[64];
        // uint32_t width, height;
        // ap_GetImageFormat(_handle, &width, &height, image_type,
            // sizeof (image_type));
        // printf("im type %s \n", image_type);



        std::cout <<"L2"<< std::endl;
        return true;
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

        std::cout << "ALLOCATED : " << _buffer_size_min << std::endl;
        for (int i = 0; i < _buffer_num; ++i) {
            _buffer.AllocateBuffer(_buffer_size_min);
        }
        std::cout << "H4" << std::endl;
    }
}  // namespace vidre
