#pragma once

#include <camera/camera.hpp>

#include <atomic>
#include <string>
#include <thread>
#include <vector>

namespace vidre {
    class DummyCamera : public ICamera {
    public:
        DummyCamera();
        ~DummyCamera();
        void SetMaxNumBuffer(unsigned int newMaxNumBuffer);

        void Init() override;
        bool Init(std::string serial) override;
        void Arm() override;
        void Disarm() override;
        void Start() override;
        void Stop() override;

        bool IsArmed() override;
        bool IsRunning() override;

        void SetSharedMemory(boost::interprocess::managed_shared_memory* sharedMemory,
                             std::vector<void *>* ptrs,
                             std::string memory_name) override;
        const SharedFrameHeader *RetrieveLatestFrame() override;

        bool SaveConfiguration(std::string filepath) override;
        bool LoadConfiguration(std::string filepath) override;

        unsigned int GetFrameWidth() override;
        unsigned int GetFrameHeight() override;
        unsigned int GetFrameOffsetX() override;
        unsigned int GetFrameOffsetY() override;
        unsigned int GetMaxNumBuffer() override;
        unsigned int GetFrameBytes() override;
        unsigned int GetBufferSize() override;
        double GetFrameRate() override;
        std::string GetSerialNumber() override;
        std::string GetVendorName() override;
        void SetFrameWidth(unsigned int newFrameWidth) override;
        void SetFrameHeight(unsigned int newFrameHeight) override;
        void SetFrameOffsetX(unsigned int newFrameOffsetX) override;
        void SetFrameOffsetY(unsigned int newFrameOffsetY) override;
        void SetFrameRate(double newFrameRate) override;
        double GetExposure() override;
        double GetGain() override;
        void SetExposure(double newExposure) override;
        void SetGain(double newGain) override;

        void SetTriggerIn(unsigned int line) override;
        void SetTriggerOut(unsigned int line) override;
        uint8_t* RGBFrame(uint8_t* data, uint32_t* w, uint32_t* h, uint32_t* d) override;

    private:
        void run();
        void allocateBuffers();
        void releaseBuffers();

        std::thread grabber;
        std::atomic<bool> running;
        std::atomic<bool> armed;
        std::atomic<SharedFrameHeader*> latestFrame;

        boost::interprocess::managed_shared_memory* sharedMemory;
        std::vector<void*>* sharedMemoryPtrs;
        std::vector<void*> allocatedBuffers;

        unsigned int frameWidth;
        unsigned int frameHeight;
        unsigned int offsetX;
        unsigned int offsetY;
        unsigned int maxNumBuffer;
        double frameRate;
        double exposure;
        double gain;
        std::string serialNumber;
        std::string vendorName;
    };
}  // namespace vidre