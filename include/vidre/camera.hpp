#pragma once
#include <string>
#include <memory>

#include "shared_frame_header.hpp"
#include "shared_frame_buffer_controller.hpp"
#include <boost/interprocess/managed_shared_memory.hpp>

namespace vidre {
    // Aptina camera does not seem to support GenICam cameras. Is adapting Aptina to
    // GenICam interface a valid and reliable option?

    // Might be needed adding another inheritance layer between
    // ICamera and PylonCamera: GenICamera. That way mainly the
    // aspects around camera parameters can be combined.

    class ICameraConfig {
    public:
        virtual void SetBool(std::string parameter, bool value) = 0;
        virtual void SetInt(std::string parameter, int64_t value) = 0;
        virtual void SetFloat(std::string parameter, float value) = 0;
        virtual bool GetBool(std::string parameter) = 0;
        virtual int64_t GetInt(std::string parameter) = 0;
        virtual float GetFloat(std::string parameter) = 0;

        virtual bool SaveConfiguration(std::string filepath) = 0;
        virtual bool LoadConfiguration(std::string filepath) = 0;

        virtual unsigned int GetFrameWidth() { return 0; }
        virtual unsigned int GetFrameHeight() { return 0; }
        virtual unsigned int GetFrameOffsetX() { return 0; }
        virtual unsigned int GetFrameOffsetY() { return 0; }
        virtual double GetFrameRate() { return 0.0; }
        virtual std::string GetSerialNumber() { return ""; }
        virtual void SetFrameWidth(unsigned int newFrameWidth) {}
        virtual void SetFrameHeight(unsigned int newFrameHeight) {}
        virtual void SetFrameOffsetX(unsigned int newFrameOffsetX) {}
        virtual void SetFrameOffsetY(unsigned int newFrameOffsetY) {}
        virtual void SetFrameRate(double newFrameRate) {}

        virtual void SetTriggerIn(unsigned int line) {}
        virtual void SetTriggerOut(unsigned int line) {}
    };

    class ICamera {
    public:
        // virtual ~ICamera() = default;

        // -------------------
        // Runtime controls
        // -------------------

        virtual void Init() = 0;
        virtual bool Init(std::string serial) = 0;
        virtual void Arm() = 0;
        virtual void Disarm() = 0;
        virtual void Start() = 0;
        virtual void Stop() = 0;

        virtual bool IsArmed() = 0;
        virtual bool IsRunning() = 0;

        virtual void SetSharedMemory(boost::interprocess::managed_shared_memory* shared_memory, std::string memory_name) = 0;
        virtual const SharedFrameHeader *RetrieveLatestFrame() = 0;

        virtual ICameraConfig *GetCameraConfig() = 0;
        virtual int64_t GetMaxNumBuffer() { return 0; }
    };
}  // namespace vidre