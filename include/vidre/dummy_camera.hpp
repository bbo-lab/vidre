#pragma once
#include "camera.hpp"

namespace vidre {
    // Currently using explicit Pylon Camera. Can it be replace by the more generic GenICam or USB3Vision stuff?

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

        virtual void SetSharedMemory(boost::interprocess::managed_shared_memory* shared_memory)
        {
            return;
        };

        virtual const SharedFrameHeader *RetrieveLatestFrame() {
            return nullptr;
        };

        virtual bool SaveConfiguration(std::string filepath) {
            return true;
        };
        virtual bool LoadConfiguration(std::string filepath) {
            return true;
        };

        // For convenience.
        virtual unsigned int GetFrameWidth() {
            return 1;
        };
        virtual unsigned int GetFrameHeight() {
            return 1;
        };
        virtual unsigned int GetFrameOffsetX() {
            return 0;
        };
        virtual unsigned int GetFrameOffsetY() {
            return 0;
        };
        virtual unsigned int GetMaxNumBuffer() {
            return 0;
        };
        virtual double GetFrameRate() {
            return 1;
        };
        virtual void SetFrameWidth(unsigned int newFrameWidth) {};
        virtual void SetFrameHeight(unsigned int newFrameHeight) {};
        virtual void SetFrameOffsetX(unsigned int newFrameOffsetX) {};
        virtual void SetFrameOffsetY(unsigned int newFrameOffsetY) {};
        virtual void SetFrameRate(double newFrameRate) {};

    private:
        std::atomic<bool> armed;
        std::atomic<bool> running;
    };
}  // namespace vidre