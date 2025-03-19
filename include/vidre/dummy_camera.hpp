#pragma once
#include "camera.hpp"

namespace vidre {
    // Currently using explicit Pylon Camera. Can it be replace by the more generic GenICam or USB3Vision stuff?

    class DummyCameraConfig : public ICameraConfig {
    public:
        DummyCameraConfig() {};
        
        void SetBool(std::string parameter, bool value) override {}
        void SetInt(std::string parameter, int64_t value) override {}
        void SetFloat(std::string parameter, float value) override {}
        bool GetBool(std::string parameter) override { return false; }
        int64_t GetInt(std::string parameter) override { return 0; }
        float GetFloat(std::string parameter) override { return 0.0; }

        bool SaveConfiguration(std::string filepath) override { return true; }
        bool LoadConfiguration(std::string filepath) override { return true; }
    };

    class DummyCamera : public ICamera {
    public:
        DummyCamera() {};
        ~DummyCamera() {};

        virtual void Init() {};
        virtual bool Init(std::string serial) {
            return true;
        };
        virtual void Arm() {
            armed = true;
        };
        virtual void Disarm() {
            armed = false;
        }
        virtual void Start() {
            running = true;
        };
        virtual void Stop() {
            running = false;
        };

        virtual bool IsArmed() {
            return armed;
        };
        virtual bool IsRunning() {
            return running;
        };

        virtual void SetSharedMemory(boost::interprocess::managed_shared_memory* shared_memory, std::string memory_name)
        {
            return;
        };

        virtual const SharedFrameHeader *RetrieveLatestFrame() {
            return nullptr;
        };

        virtual ICameraConfig *GetCameraConfig() { return cameraConfig.get(); }

    private:
        std::atomic<bool> armed;
        std::atomic<bool> running;

        std::unique_ptr<DummyCameraConfig> cameraConfig;
    };
}  // namespace vidre