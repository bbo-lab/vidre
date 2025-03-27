#pragma once
#include "camera.hpp"

#include <thread>

// #ifdef PYLON_WIN_BUILD
#include <pylon/PylonGUI.h>
// #endif
#include <pylon/PylonIncludes.h>
#include "pylon_shr_mem_buffer_factory.hpp"

namespace vidre {
    class PylonCameraConfig : public ICameraConfig {
    public:
        PylonCameraConfig();

        void SetNodeMap(GenApi::INodeMap &newNodeMap);
        GenApi::INodeMap *GetNodeMap();

        virtual void SetBool(std::string parameter, bool value) override;
        virtual void SetInt(std::string parameter, int64_t value) override;
        virtual void SetFloat(std::string parameter, double value) override;
        virtual void SetString(std::string parameter, std::string value) override;
        virtual void SetEnum(std::string parameter, std::string value) override;
        virtual bool GetBool(std::string parameter) override;
        virtual int64_t GetInt(std::string parameter) override;
        virtual double GetFloat(std::string parameter) override;
        virtual std::string GetString(std::string parameter) override;
        virtual std::string GetEnum(std::string parameter) override;

        virtual bool SaveConfiguration(std::string filepath) override;
        virtual bool LoadConfiguration(std::string filepath) override;

        virtual int64_t GetFrameWidth() override;
        virtual int64_t GetFrameHeight() override;
        virtual int64_t GetFrameOffsetX() override;
        virtual int64_t GetFrameOffsetY() override;
        virtual double GetFrameRate() override;
        virtual std::string GetSerialNumber();
        virtual void SetFrameWidth(int64_t newFrameWidth) override;
        virtual void SetFrameHeight(int64_t newFrameHeight) override;
        virtual void SetFrameOffsetX(int64_t newFrameOffsetX) override;
        virtual void SetFrameOffsetY(int64_t newFrameOffsetY) override;
        virtual void SetFrameRate(double newFrameRate) override;

        virtual void SetTriggerIn(int64_t line) override;
        virtual void SetTriggerOut(int64_t line) override;
    private:
        GenApi::INodeMap *nodeMap;
    };

    class PylonCamera : public ICamera, public Pylon::CImageEventHandler {
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

        virtual ICameraConfig &GetCameraConfig() override;
        virtual int64_t GetMaxNumBuffer() override;
    private:
        std::atomic<bool> armed;

        std::atomic<SharedFrameHeader *> latestFrame;

        // -------- Pylon specific --------
        virtual void OnImageGrabbed( Pylon::CInstantCamera& /*camera*/, const Pylon::CGrabResultPtr& /*ptrGrabResult*/ ) override;
        void _Init();

        Pylon::CInstantCamera camera;
        Pylon::PylonAutoInitTerm pylonAutoInitTerm;
        ShrBufferFactory shrBufferFactory;
        PylonCameraConfig cameraConfig;
        // --------------------------------
    };
}  // namespace vidre