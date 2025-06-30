#pragma once
#include "vidre/camera.hpp"
#include "vidre/aptina_buffer.hpp"

#include <thread>

#include "apbase.h"

namespace vidre {
    class AptinaCamera : public ICamera {
    public:
        AptinaCamera();
        ~AptinaCamera();

        virtual void Init() override;
        virtual bool Init(std::string serial) override;
        virtual void Arm() override;
        virtual void Disarm() override;
        virtual void Start() override;
        virtual void Stop() override;

        virtual bool IsArmed() override;
        virtual bool IsRunning() override;

        virtual void SetSharedMemory(boost::interprocess::managed_shared_memory* sharedMemory, std::vector<void *>* ptrs, std::string memory_name) override;
        virtual const SharedFrameHeader *RetrieveLatestFrame() override;

        virtual bool SaveConfiguration(std::string filepath) override;
        virtual bool LoadConfiguration(std::string filepath) override;


        // For convenience.
        virtual unsigned int GetFrameWidth() override;
        virtual unsigned int GetFrameHeight() override;
        virtual unsigned int GetFrameOffsetX() override;
        virtual unsigned int GetFrameOffsetY() override;
        virtual unsigned int GetMaxNumBuffer() override;
        virtual double GetFrameRate() override;
        virtual std::string GetSerialNumber() override;
        virtual void SetFrameWidth(unsigned int newFrameWidth) override;
        virtual void SetFrameHeight(unsigned int newFrameHeight) override;
        virtual void SetFrameOffsetX(unsigned int newFrameOffsetX) override;
        virtual void SetFrameOffsetY(unsigned int newFrameOffsetY) override;
        virtual void SetFrameRate(double newFrameRate) override;

        virtual void SetTriggerIn(unsigned int line) override;
        virtual void SetTriggerOut(unsigned int line) override;
    private:
        AP_HANDLE _handle;
        
        void run();

        std::thread grabber;
        std::atomic<bool> running;

        std::atomic<bool> armed;

        std::atomic<SharedFrameHeader *> latestFrame;

        int64_t retrievedFramesNumber = 0;

        AptinaBuffer _buffer;
        unsigned int _buffer_size = 0;
        unsigned int _buffer_num = 500;

        unsigned int _w;
        unsigned int _h;
    };
}  // namespace vidre