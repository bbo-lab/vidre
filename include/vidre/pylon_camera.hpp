#pragma once
#include "camera.hpp"

#include <thread>

// #ifdef PYLON_WIN_BUILD
#include <pylon/PylonGUI.h>
// #endif
#include <pylon/PylonIncludes.h>
#include "pylon_shr_mem_buffer_factory.hpp"

namespace vidre {
    // Currently using explicit Pylon Camera. Can it be replace by the more generic GenICam or USB3Vision stuff?

    class PylonCameraConfig : public ICameraConfig {
    public:
        PylonCameraConfig() = delete;
        PylonCameraConfig(Pylon::CInstantCamera camera);

        void SetNodeMap(GenApi::INodeMap &newNodeMap);
        GenApi::INodeMap &GetNodeMap();

        virtual void SetBool(std::string parameter, bool value) override;
        virtual void SetInt(std::string parameter, int64_t value) override;
        virtual void SetFloat(std::string parameter, float value) override;
        virtual bool GetBool(std::string parameter) override;
        virtual int64_t GetInt(std::string parameter) override;
        virtual float GetFloat(std::string parameter) override;

        virtual bool SaveConfiguration(std::string filepath) override;
        virtual bool LoadConfiguration(std::string filepath) override;

        unsigned int GetFrameWidth();
        unsigned int GetFrameHeight();
        unsigned int GetFrameOffsetX();
        unsigned int GetFrameOffsetY();
        unsigned int GetMaxNumBuffer();
        double GetFrameRate();
        std::string GetSerialNumber();
        void SetFrameWidth(unsigned int newFrameWidth);
        void SetFrameHeight(unsigned int newFrameHeight);
        void SetFrameOffsetX(unsigned int newFrameOffsetX);
        void SetFrameOffsetY(unsigned int newFrameOffsetY);
        void SetFrameRate(double newFrameRate);
    
        void SetTriggerIn(unsigned int line);
        void SetTriggerOut(unsigned int line);
    private:
        GenApi::INodeMap &nodeMap;
    };

    class PylonCamera : public ICamera {
    public:
        PylonCamera();
        ~PylonCamera();

        virtual void Init() override;
        virtual bool Init(std::string serial) override;
        virtual void Arm() override;
        virtual void Disarm() override;
        virtual void Start() override;
        virtual void Stop() override;

        virtual bool IsArmed() override;
        virtual bool IsRunning() override;

        virtual void SetSharedMemory(boost::interprocess::managed_shared_memory* shared_memory, std::string memory_name) override;
        virtual const SharedFrameHeader *RetrieveLatestFrame() override;

        ICameraConfig *GetCameraConfig();
    private:
        void run();

        std::thread grabber;
        std::atomic<bool> running;

        std::atomic<bool> armed;

        std::atomic<SharedFrameHeader *> latestFrame;

        // -------- Pylon specific --------
        Pylon::CInstantCamera camera;
        Pylon::PylonAutoInitTerm pylonAutoInitTerm;
        ShrBufferFactory shrBufferFactory;
        std::unique_ptr<PylonCameraConfig> cameraConfig;
        // --------------------------------
    };
}  // namespace vidre