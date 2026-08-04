#pragma once

#include <memory>
#include <string>

#include <camera/camera.hpp>
#include <core/transport/wire_protocol.hpp>

namespace vidre {

    class ControlClient;

    class RemoteCamera : public ICamera {
    public:
        RemoteCamera(std::string name, transport::CameraInfo info, std::shared_ptr<ControlClient> control);

        void Init() override {}
        bool Init(std::string serial) override { return true; }
        void Arm() override {}
        void Disarm() override {}
        void Start() override {}
        void Stop() override {}

        bool IsArmed() override { return _status.armed; }
        bool IsRunning() override { return _status.running; }

        void SetSharedMemory(boost::interprocess::managed_shared_memory*, std::vector<void*>*, std::string) override {}
        const SharedFrameHeader* RetrieveLatestFrame() override { return nullptr; }

        bool SaveConfiguration(std::string) override { return false; }
        bool LoadConfiguration(std::string) override { return true; }

        unsigned int GetFrameWidth() override { return _info.width; }
        unsigned int GetFrameHeight() override { return _info.height; }
        unsigned int GetFrameOffsetX() override { return 0; }
        unsigned int GetFrameOffsetY() override { return 0; }
        unsigned int GetMaxNumBuffer() override { return _info.maxNumBuffers; }
        unsigned int GetFrameBytes() override { return _info.frameBytes; }
        unsigned int GetBufferSize() override { return _info.bufferSize; }
        double GetFrameRate() override { return _info.frameRate; }
        std::string GetSerialNumber() override { return _info.serialNumber; }
        std::string GetVendorName() override { return _info.vendorName; }

        void SetFrameWidth(unsigned int) override {}
        void SetFrameHeight(unsigned int) override {}
        void SetFrameOffsetX(unsigned int) override {}
        void SetFrameOffsetY(unsigned int) override {}
        void SetFrameRate(double) override {}

        double GetExposure() override { return _info.exposure; }
        double GetGain() override { return _info.gain; }
        void SetExposure(double newExposure) override;
        void SetGain(double newGain) override;

        void SetTriggerIn(unsigned int) override {}
        void SetTriggerOut(unsigned int) override {}
        uint8_t* RGBFrame(uint8_t*, uint32_t*, uint32_t*, uint32_t*) override { return nullptr; }

        bool HadError() override { return _status.error; }

        void updateStatus(const transport::CameraStatus& status) { _status = status; }

    private:
        std::string _name;
        transport::CameraInfo _info;
        transport::CameraStatus _status;
        std::shared_ptr<ControlClient> _control;
    };

}  // namespace vidre
