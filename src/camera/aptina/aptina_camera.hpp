#pragma once
#include <camera/camera.hpp>
#include <camera/aptina/aptina_buffer.hpp>

#include <optional>
#include <thread>
#include <unordered_map>
#include <vector>

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
        virtual unsigned int GetFrameBytes() override;
        virtual unsigned int GetBufferSize() override;
        virtual double GetFrameRate() override;
        virtual std::string GetSerialNumber() override;
        virtual std::string GetVendorName() override;
        virtual void SetFrameWidth(unsigned int newFrameWidth) override;
        virtual void SetFrameHeight(unsigned int newFrameHeight) override;
        virtual void SetFrameOffsetX(unsigned int newFrameOffsetX) override;
        virtual void SetFrameOffsetY(unsigned int newFrameOffsetY) override;
        virtual void SetFrameRate(double newFrameRate) override;
        virtual double GetExposure() override;
        virtual double GetGain() override;
        virtual void SetExposure(double newExposure) override;
        virtual void SetGain(double newGain) override;

        virtual void SetTriggerIn(unsigned int line) override;
        virtual void SetTriggerOut(unsigned int line) override;
        virtual uint8_t* RGBFrame(uint8_t* data, uint32_t* w, uint32_t* h, uint32_t* d) override;
        virtual bool HadError() override { return _hadError.load(std::memory_order_relaxed); }
    private:
        AP_HANDLE _handle;
        
        void run();

        std::thread grabber;
        std::atomic<bool> running;

        std::atomic<bool> armed;

        std::atomic<bool> _hadError{false};

        std::atomic<SharedFrameHeader *> latestFrame;

        int64_t retrievedFramesNumber = 0;

        AptinaBuffer _buffer;
        unsigned int _buffer_size_min = 0;
        unsigned int _buffer_size_save = 0;
        unsigned int _buffer_num = 500;

        unsigned int _w;
        unsigned int _h;


        std::string _serial;
        std::string _config_file;

        std::optional<std::string> FindRegisterSymbol(const std::vector<std::string>& preferredSymbols,
                                                      const std::vector<std::string>& containsTokens);
        double GetRegisterValue(const std::vector<std::string>& preferredSymbols,
                                const std::vector<std::string>& containsTokens);
        bool SetRegisterValue(const std::vector<std::string>& preferredSymbols,
                              const std::vector<std::string>& containsTokens,
                              double value,
                              const char* valueLabel);
        std::unordered_map<std::string, std::string> _resolvedRegisterSymbols;

        std::vector<std::string> _exposurePreferredSymbols;
        std::vector<std::string> _exposureContainsTokens;
        std::vector<std::string> _gainPreferredSymbols;
        std::vector<std::string> _gainContainsTokens;
    };
}  // namespace vidre