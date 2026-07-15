// #include "aptina_api_loader.hpp"
#include <camera/aptina/aptina_camera.hpp>

#include <algorithm>
#include <cctype>
#include <cmath>
#include <iostream>


// TODO: check whether if(ap_F) is needed

namespace vidre {
    namespace {
        std::string ToLower(std::string value) {
            std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) {
                return static_cast<char>(std::tolower(c));
            });
            return value;
        }

        bool IsAptinaSuccess(int result) {
            return result == AP_CAMERA_SUCCESS || result == AP_INI_SUCCESS;
        }
    }  // namespace

    AptinaCamera::AptinaCamera() :
        running(false),
        armed(false),
        _handle(nullptr),
        _exposurePreferredSymbols({"coarse_integration_time", "integration_time", "exposure", "xstdat"}),
        _exposureContainsTokens({"coarse_integration", "integration", "exposure", "xstdat", "shutter"}),
        _gainPreferredSymbols({"analog_gain", "global_gain", "gain", "analoggain"}),
        _gainContainsTokens({"analog_gain", "global_gain", "gain"}) {}
    AptinaCamera::~AptinaCamera() {
        if (ap_Finalize) ap_Finalize();
    }

    void AptinaCamera::run() {
        running.store(true);
        while (running.load()) {
            auto [buffer, header] = _buffer.CurrentBuffer(_buffer_num);
            if (!ap_GrabFrame || !ap_GetLastError) {
                std::cerr << "Aptina API not loaded!\n";
                _hadError.store(true, std::memory_order_release);
                running.store(false);
                return;
            }
            ap_GrabFrame(_handle, buffer, _buffer_size_min);
            auto computerTimeStamp = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now() - vidre::StartTime
            ).count();
            ++retrievedFramesNumber;
            auto success = ap_GetLastError();
            if (success != AP_CAMERA_SUCCESS) {
                std::cout << "APTINA FAILED TO GRAB\n";
                _hadError.store(true, std::memory_order_release);
                running.store(false);
                return;
            }

            header->timeStamp = computerTimeStamp;
            header->computerTimeStamp = computerTimeStamp;
            header->frameID.store(retrievedFramesNumber, std::memory_order_release);

            latestFrame.store(header, std::memory_order_release);

            if (frameSemaphore) {
                frameSemaphore->release();
            }
        }
        std::cout << "RETRIEVED : " << retrievedFramesNumber << std::endl;
        running.store(false);
    }

    void AptinaCamera::Init() {
        if (!ap_Create) {
            std::cerr << "Aptina API not loaded!\n";
            return;
        }
        _handle = ap_Create(0);
        if (_handle == nullptr) {
            std::cout << "Failed to Init aptina\n";
        }
    }

    bool AptinaCamera::Init(std::string serial) {
        _serial = serial;
        if (!ap_Create || !ap_SetState) {
            std::cerr << "Aptina API not loaded!\n";
            return false;
        }
        _handle = ap_Create(0);
        if (_handle == nullptr) {
            std::cout << "Failed to Init aptina\n";
            return false;
        }
        ap_SetState(_handle, "Sensor Reset", 1);
        #ifdef _WIN32
            Sleep(1000);
        #else
            sleep(1);
        #endif
            ap_SetState(_handle, "Sensor Reset", 0);
        #ifdef _WIN32
            Sleep(1000);
        #else
            sleep(1);
        #endif
        return true;
    }

    void AptinaCamera::Arm() {
    }
    void AptinaCamera::Disarm() {
        _buffer.Reset();
        retrievedFramesNumber = 0;
        frameSemaphore = nullptr;
        latestFrame.store(nullptr, std::memory_order_release);
    }
    void AptinaCamera::Start() {
        latestFrame.store(nullptr, std::memory_order_release);
        _hadError.store(false, std::memory_order_release);
        grabber = std::thread(&AptinaCamera::run, this);
    }
    void AptinaCamera::Stop() {
        running.store(false);
        if (grabber.joinable())
            grabber.join();
    }

    const SharedFrameHeader* AptinaCamera::RetrieveLatestFrame() {
        return latestFrame.load();
    }
    bool AptinaCamera::IsArmed() {
        return armed.load();
    }
    bool AptinaCamera::IsRunning() {
        return running.load();
    }

    unsigned int AptinaCamera::GetFrameWidth() {
       return _w;
    }

    unsigned int AptinaCamera::GetFrameHeight() {
        return _h;
    }

    unsigned int AptinaCamera::GetFrameOffsetX() {
        return 0;
    }

    unsigned int AptinaCamera::GetFrameOffsetY() {
        return 0;
    }

    unsigned int AptinaCamera::GetMaxNumBuffer() {
        return _buffer_num;
    }
    unsigned int AptinaCamera::GetFrameBytes() {
        return _buffer_size_save;
    }
    unsigned int AptinaCamera::GetBufferSize() {
        return _buffer_size_min;
    }



    double AptinaCamera::GetFrameRate() {
        return 50;
    }

    std::string AptinaCamera::GetSerialNumber() {
        return _serial;
    }

    std::string AptinaCamera::GetVendorName() {
        return "aptina";
    }

    void AptinaCamera::SetFrameWidth(unsigned int newFrameWidth) {
       
    }

    void AptinaCamera::SetFrameHeight(unsigned int newFrameHeight) {
    }

    void AptinaCamera::SetFrameOffsetX(unsigned int newFrameOffsetX) {
    }

    void AptinaCamera::SetFrameOffsetY(unsigned int newFrameOffsetY) {
    }

    void AptinaCamera::SetFrameRate(double newFrameRate) {
    }

    std::optional<std::string> AptinaCamera::FindRegisterSymbol(const std::vector<std::string>& preferredSymbols,
                                                                const std::vector<std::string>& containsTokens) {
        std::string cacheKey = preferredSymbols.empty() ? "fallback" : preferredSymbols.front();
        auto cacheMatch = _resolvedRegisterSymbols.find(cacheKey);
        if (cacheMatch != _resolvedRegisterSymbols.end()) {
            return cacheMatch->second;
        }

        if (_handle == nullptr || !ap_NumRegisters || !ap_Register || !ap_RegisterSymbol || !ap_DestroyRegister) {
            return std::nullopt;
        }

        std::vector<std::pair<std::string, std::string>> registerSymbols;
        const int registerCount = ap_NumRegisters(_handle, nullptr);
        registerSymbols.reserve(registerCount);

        for (int i = 0; i < registerCount; ++i) {
            AP_HANDLE hReg = ap_Register(_handle, nullptr, i);
            if (hReg == nullptr) {
                continue;
            }

            const char* symbolRaw = ap_RegisterSymbol(hReg);
            if (symbolRaw != nullptr && symbolRaw[0] != '\0') {
                std::string symbol(symbolRaw);
                registerSymbols.emplace_back(symbol, ToLower(symbol));
            }
            ap_DestroyRegister(hReg);
        }

        for (const auto& preferred : preferredSymbols) {
            const std::string preferredLower = ToLower(preferred);
            for (const auto& [symbol, symbolLower] : registerSymbols) {
                if (symbolLower == preferredLower) {
                    _resolvedRegisterSymbols[cacheKey] = symbol;
                    return symbol;
                }
            }
        }

        for (const auto& preferred : preferredSymbols) {
            const std::string preferredLower = ToLower(preferred);
            for (const auto& [symbol, symbolLower] : registerSymbols) {
                if (symbolLower.find(preferredLower) != std::string::npos) {
                    _resolvedRegisterSymbols[cacheKey] = symbol;
                    return symbol;
                }
            }
        }

        for (const auto& token : containsTokens) {
            const std::string tokenLower = ToLower(token);
            for (const auto& [symbol, symbolLower] : registerSymbols) {
                if (symbolLower.find(tokenLower) != std::string::npos) {
                    _resolvedRegisterSymbols[cacheKey] = symbol;
                    return symbol;
                }
            }
        }

        return std::nullopt;
    }

    double AptinaCamera::GetRegisterValue(const std::vector<std::string>& preferredSymbols,
                                          const std::vector<std::string>& containsTokens) {
        if (_handle == nullptr || !ap_GetSensorRegister) {
            return 0.0;
        }

        auto regSymbol = FindRegisterSymbol(preferredSymbols, containsTokens);
        if (!regSymbol.has_value()) {
            return 0.0;
        }

        ap_u32 value = 0;
        const int result = ap_GetSensorRegister(_handle, regSymbol->c_str(), nullptr, &value, false);
        if (!IsAptinaSuccess(result)) {
            std::cout << "Failed to get sensor register: " << *regSymbol << std::endl;
            return 0.0;
        }

        return static_cast<double>(value);
    }

    bool AptinaCamera::SetRegisterValue(const std::vector<std::string>& preferredSymbols,
                                        const std::vector<std::string>& containsTokens,
                                        double value,
                                        const char* valueLabel) {
        if (_handle == nullptr || !ap_SetSensorRegister) {
            return false;
        }

        auto regSymbol = FindRegisterSymbol(preferredSymbols, containsTokens);
        if (!regSymbol.has_value()) {
            std::cout << "Could not resolve " << valueLabel << " register symbol" << std::endl;
            return false;
        }

        ap_u32 oldValue = 0;
        ap_u32 sideEffects = 0;
        ap_u32 newValue = static_cast<ap_u32>(max(0.0, std::round(value)));

        if (ap_TestSetSensorRegister) {
            sideEffects = ap_TestSetSensorRegister(_handle, regSymbol->c_str(), newValue, 0, &oldValue);
        }

        const int result = ap_SetSensorRegister(_handle, regSymbol->c_str(), nullptr, newValue, nullptr);
        if (!IsAptinaSuccess(result)) {
            std::cout << "Failed to set " << valueLabel << " register: " << *regSymbol << std::endl;
            return false;
        }

        if (sideEffects & AP_FLAG_REALLOC) {
            std::cout << "Warning: " << valueLabel << " register write requested frame reallocation side-effects" << std::endl;
        }

        return true;
    }

    double AptinaCamera::GetExposure() {
        return GetRegisterValue(_exposurePreferredSymbols, _exposureContainsTokens);
    }

    double AptinaCamera::GetGain() {
        return GetRegisterValue(_gainPreferredSymbols, _gainContainsTokens);
    }
    
    void AptinaCamera::SetExposure(double newExposure) { 
        SetRegisterValue(_exposurePreferredSymbols, _exposureContainsTokens, newExposure, "exposure");
    }

    void AptinaCamera::SetGain(double newGain) {
        SetRegisterValue(_gainPreferredSymbols, _gainContainsTokens, newGain, "gain");
    }
    
    void AptinaCamera::SetTriggerIn(unsigned int line) {
        if (!ap_Destroy || !ap_Finalize || !ap_LoadIniPreset) {
            std::cerr << "Aptina API not loaded!\n";
            return;
        }
        ap_Destroy(_handle);
        ap_Finalize();
        Init(_serial);
        int result = ap_LoadIniPreset(_handle, _config_file.c_str(), "EyeCAM-Triggered");
        if (result != AP_INI_SUCCESS) {
            std::cout << "Failed to load trigger config for aptina\n";
            return;
        }
    }
    
    void AptinaCamera::SetTriggerOut(unsigned int line) {
    }

    bool AptinaCamera::LoadConfiguration(std::string filepath) {
        if (!ap_LoadIniPreset || !ap_CheckSensorState || !ap_GetImageFormat || !ap_GrabFrame || !ap_GetLastError) {
            std::cerr << "Aptina API not loaded!\n";
            return false;
        }
        int result = ap_LoadIniPreset(_handle, filepath.c_str(), "EyeCAM");
        if (result != AP_INI_SUCCESS) {
            std::cout << "Failed to load config for  aptina\n";
            return false;
        }
        Sleep(500);
        ap_CheckSensorState(_handle, 0);
        ap_GetImageFormat(_handle, &_w, &_h, nullptr, 0);
        _buffer_size_min = ap_GrabFrame(_handle, nullptr, 0);
        auto success = ap_GetLastError();
        if (success != AP_CAMERA_SUCCESS) {
            std::cout << "APTINA FAILED TO GRAB 1 \n";
            return false;
        }
        uint8_t* b = new uint8_t[_buffer_size_min * 2];
        for (auto cnt = 0; cnt < 100; ++cnt) {
            _buffer_size_save = ap_GrabFrame(_handle, b, 2 * _buffer_size_min);
            success = ap_GetLastError();
            if (success == AP_CAMERA_SUCCESS) {
                break;
            }
        }
        delete[] b;
        b = NULL;

        _config_file = filepath;

        return true;
    }

    bool AptinaCamera::SaveConfiguration(std::string filepath) {
        return true;
    }

    void AptinaCamera::SetSharedMemory(boost::interprocess::managed_shared_memory* sharedMemory, std::vector<void *>* ptrs, std::string memory_name, std::counting_semaphore<>* semaphore)
    {
        _buffer.SetSharedMemory(sharedMemory, ptrs, memory_name);
        frameSemaphore = semaphore;

        for (int i = 0; i < _buffer_num; ++i) {
            _buffer.AllocateBuffer(_buffer_size_min);
        }
    }

    uint8_t* AptinaCamera::RGBFrame(uint8_t* data, uint32_t* w, uint32_t* h, uint32_t* d) {
        return ap_ColorPipe(_handle, data, _buffer_size_min, w, h, d);
    }
}  // namespace vidre


extern "C" {

VIDRE_CAMERA_API vidre::ICamera* create_aptina_camera() {
    return new vidre::AptinaCamera();
}

VIDRE_CAMERA_API void destroy_aptina_camera(vidre::ICamera* camera) {
    delete camera;
}
}