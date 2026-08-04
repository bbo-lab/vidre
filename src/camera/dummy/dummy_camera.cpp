#include <camera/dummy/dummy_camera.hpp>

#include <core/runtime/shared_frame_header.hpp>

#include <algorithm>
#include <chrono>
#include <cstring>
#include <iostream>
#include <new>
#include <utility>

namespace vidre {
    DummyCamera::DummyCamera()
        : running(false),
          armed(false),
          latestFrame(nullptr),
          sharedMemory(nullptr),
          sharedMemoryPtrs(nullptr),
          frameWidth(1280),
          frameHeight(720),
          offsetX(0),
          offsetY(0),
          maxNumBuffer(500),
          frameRate(0.0),
          exposure(0.0),
          gain(0.0),
          serialNumber("dummy-camera"),
          vendorName("dummy") {}

    DummyCamera::~DummyCamera() {
        Stop();
    }

    void DummyCamera::Init() {}

    bool DummyCamera::Init(std::string serial) {
        serialNumber = std::move(serial);
        return true;
    }

    void DummyCamera::SetMaxNumBuffer(unsigned int newMaxNumBuffer) {
        maxNumBuffer = newMaxNumBuffer;
    }

    void DummyCamera::allocateBuffers() {
        if (sharedMemory == nullptr || sharedMemoryPtrs == nullptr || !allocatedBuffers.empty()) {
            return;
        }

        const size_t frameBytes = GetFrameBytes();
        const size_t bufferBytes = sizeof(SharedFrameHeader) + frameBytes;

        allocatedBuffers.reserve(maxNumBuffer);
        for (unsigned int index = 0; index < maxNumBuffer; ++index) {
            void* allocatedMemory = sharedMemory->allocate(bufferBytes);
            new (allocatedMemory) SharedFrameHeader{};

            auto* frameBytesPtr = static_cast<uint8_t*>(allocatedMemory) + sizeof(SharedFrameHeader);
            std::fill(frameBytesPtr, frameBytesPtr + frameBytes, 0);

            sharedMemoryPtrs->at(index) = allocatedMemory;
            allocatedBuffers.push_back(allocatedMemory);
        }
    }

    void DummyCamera::releaseBuffers() {
        if (sharedMemory == nullptr) {
            allocatedBuffers.clear();
            return;
        }

        for (void* buffer : allocatedBuffers) {
            sharedMemory->deallocate(buffer);
        }
        allocatedBuffers.clear();
    }

    void DummyCamera::Arm() {
        allocateBuffers();
        armed.store(true);
    }

    void DummyCamera::Disarm() {
        armed.store(false);
        releaseBuffers();
        latestFrame.store(nullptr);
    }

    void DummyCamera::run() {
        running.store(true);
        uint64_t frameCounter = 0;

        while (running.load()) {
            if (sharedMemoryPtrs == nullptr || sharedMemoryPtrs->empty()) {
                std::this_thread::yield();
                continue;
            }

            const auto slot = static_cast<size_t>(frameCounter % maxNumBuffer);
            void* buffer = sharedMemoryPtrs->at(slot);
            if (buffer == nullptr) {
                std::this_thread::yield();
                continue;
            }

            auto* header = static_cast<SharedFrameHeader*>(buffer);
            auto* pixels = static_cast<uint8_t*>(buffer) + sizeof(SharedFrameHeader);
            const size_t frameBytes = GetFrameBytes();

            const uint8_t value = static_cast<uint8_t>(frameCounter & 0xFF);
            std::fill(pixels, pixels + frameBytes, value);

            const auto now = std::chrono::system_clock::now();
            const auto timeSinceStart = std::chrono::duration_cast<std::chrono::milliseconds>(now - StartTime).count();

            header->timeStamp = static_cast<uint64_t>(timeSinceStart);
            header->computerTimeStamp = static_cast<int64_t>(timeSinceStart);
            header->frameID.store(static_cast<int64_t>(frameCounter + 1), std::memory_order_release);

            latestFrame.store(header, std::memory_order_release);
            ++frameCounter;

            if (frameRate > 0.0) {
                std::this_thread::sleep_for(std::chrono::duration<double>(1.0 / frameRate));
            } else {
                std::this_thread::yield();
            }
        }

        running.store(false);
    }

    void DummyCamera::Start() {
        if (!armed.load()) {
            return;
        }
        latestFrame.store(nullptr);
        if (grabber.joinable()) {
            return;
        }
        grabber = std::thread(&DummyCamera::run, this);
    }

    void DummyCamera::Stop() {
        running.store(false);
        if (grabber.joinable()) {
            grabber.join();
        }
    }

    bool DummyCamera::IsArmed() {
        return armed.load();
    }

    bool DummyCamera::IsRunning() {
        return running.load();
    }

    void DummyCamera::SetSharedMemory(boost::interprocess::managed_shared_memory* memory,
                                      std::vector<void *>* ptrs,
                                      std::string) {
        sharedMemory = memory;
        sharedMemoryPtrs = ptrs;
    }

    const SharedFrameHeader *DummyCamera::RetrieveLatestFrame() {
        return latestFrame.load();
    }

    bool DummyCamera::SaveConfiguration(std::string) {
        return true;
    }

    bool DummyCamera::LoadConfiguration(std::string) {
        return true;
    }

    unsigned int DummyCamera::GetFrameWidth() {
        return frameWidth;
    }

    unsigned int DummyCamera::GetFrameHeight() {
        return frameHeight;
    }

    unsigned int DummyCamera::GetFrameOffsetX() {
        return offsetX;
    }

    unsigned int DummyCamera::GetFrameOffsetY() {
        return offsetY;
    }

    unsigned int DummyCamera::GetMaxNumBuffer() {
        return maxNumBuffer;
    }

    unsigned int DummyCamera::GetFrameBytes() {
        return frameWidth * frameHeight;
    }

    unsigned int DummyCamera::GetBufferSize() {
        return GetFrameBytes();
    }

    double DummyCamera::GetFrameRate() {
        return frameRate;
    }

    std::string DummyCamera::GetSerialNumber() {
        return serialNumber;
    }

    std::string DummyCamera::GetVendorName() {
        return vendorName;
    }

    void DummyCamera::SetFrameWidth(unsigned int newFrameWidth) {
        frameWidth = newFrameWidth;
    }

    void DummyCamera::SetFrameHeight(unsigned int newFrameHeight) {
        frameHeight = newFrameHeight;
    }

    void DummyCamera::SetFrameOffsetX(unsigned int newFrameOffsetX) {
        offsetX = newFrameOffsetX;
    }

    void DummyCamera::SetFrameOffsetY(unsigned int newFrameOffsetY) {
        offsetY = newFrameOffsetY;
    }

    void DummyCamera::SetFrameRate(double newFrameRate) {
        frameRate = newFrameRate;
    }

    double DummyCamera::GetExposure() {
        return exposure;
    }

    double DummyCamera::GetGain() {
        return gain;
    }

    void DummyCamera::SetExposure(double newExposure) {
        exposure = newExposure;
    }

    void DummyCamera::SetGain(double newGain) {
        gain = newGain;
    }

    void DummyCamera::SetTriggerIn(unsigned int) {}

    void DummyCamera::SetTriggerOut(unsigned int) {}

    uint8_t* DummyCamera::RGBFrame(uint8_t* data, uint32_t* w, uint32_t* h, uint32_t* d) {
        if (w != nullptr) {
            *w = frameWidth;
        }
        if (h != nullptr) {
            *h = frameHeight;
        }
        if (d != nullptr) {
            *d = 1;
        }
        return data;
    }
}  // namespace vidre