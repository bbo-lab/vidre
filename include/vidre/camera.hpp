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

    // Camera Config should be used to split off the cameras static configurations from the runtime aspects.
    // Later useful because we can abstract GenAPI configuration stuff and reuse the implementation for the configuration
    // for different camera types that all support GenICam.
    class ICameraConfig {
    public:
        virtual void SetBool(std::string parameter, bool value) = 0;
        virtual void SetInt(std::string parameter, int64_t value) = 0;
        virtual void SetFloat(std::string parameter, double value) = 0; // GenApi clarification: The actual function is called SetFloat even though the parameter is a double.
        virtual void SetString(std::string parameter, std::string value) = 0;
        virtual void SetEnum(std::string parameter, std::string value) = 0;
        virtual bool GetBool(std::string parameter) = 0;
        virtual int64_t GetInt(std::string parameter) = 0;
        virtual double GetFloat(std::string parameter) = 0; // s.a.
        virtual std::string GetString(std::string parameter) = 0;
        virtual std::string GetEnum(std::string parameter) = 0;

        virtual bool SaveConfiguration(std::string filepath) = 0;
        virtual bool LoadConfiguration(std::string filepath) = 0;

        // Remove eventually
        virtual int64_t GetFrameWidth() { return 0; }
        virtual int64_t GetFrameHeight() { return 0; }
        virtual int64_t GetFrameOffsetX() { return 0; }
        virtual int64_t GetFrameOffsetY() { return 0; }
        virtual double GetFrameRate() { return 0.0; } 
        virtual std::string GetSerialNumber() { return ""; }
        virtual void SetFrameWidth(int64_t newFrameWidth) {}
        virtual void SetFrameHeight(int64_t newFrameHeight) {}
        virtual void SetFrameOffsetX(int64_t newFrameOffsetX) {}
        virtual void SetFrameOffsetY(int64_t newFrameOffsetY) {}
        virtual void SetFrameRate(double newFrameRate) {}

        virtual void SetTriggerIn(int64_t line) {}
        virtual void SetTriggerOut(int64_t line) {}
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

        virtual ICameraConfig &GetCameraConfig() = 0;
        virtual int64_t GetMaxNumBuffer() { return 0; }
    };
}  // namespace vidre