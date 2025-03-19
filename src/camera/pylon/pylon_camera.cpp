#include "vidre/pylon_camera.hpp"

namespace vidre {
    // class CSampleImageEventHandler : public Pylon::CImageEventHandler {
    // public:
    //     virtual void OnImageGrabbed(Pylon::CInstantCamera & /*camera*/, const Pylon::CGrabResultPtr &ptrGrabResult) {
    //         SharedFrameHeader *header = (SharedFrameHeader *)(ptrGrabResult->GetBuffer()) - 1;
    //         header->frameID = ptrGrabResult->GetImageNumber();
    //         header->timeStamp = ptrGrabResult->GetTimeStamp();
    //     }
    // };
    PylonCameraConfig::PylonCameraConfig() {
        nodeMap = GenApi::CNodeMapFactory::CreateEmptyNodeMap();
    }

        
    void PylonCameraConfig::SetNodeMap(GenApi::INodeMap &newNodeMap) {
        nodeMap = std::addressof(newNodeMap);
    }
    GenApi::INodeMap *PylonCameraConfig::GetNodeMap() {
        return nodeMap;
    }

    void PylonCameraConfig::SetBool(std::string parameter, bool value) {
        Pylon::CBooleanParameter(nodeMap, parameter.c_str()).SetValue(value);
    }
    void PylonCameraConfig::SetInt(std::string parameter, int64_t value) {
        Pylon::CIntegerParameter(nodeMap, parameter.c_str()).SetValue(value);
    } 
    void PylonCameraConfig::SetFloat(std::string parameter, float value) {
        Pylon::CFloatParameter(nodeMap, parameter.c_str()).SetValue(value);
    }
    bool PylonCameraConfig::GetBool(std::string parameter) {
        return Pylon::CBooleanParameter(nodeMap, parameter.c_str()).GetValue();
    }
    int64_t PylonCameraConfig::GetInt(std::string parameter) {
        return Pylon::CIntegerParameter(nodeMap, parameter.c_str()).GetValue();
    }
    float PylonCameraConfig::GetFloat(std::string parameter) {
        return Pylon::CFloatParameter(nodeMap, parameter.c_str()).GetValue();
    }
    

    unsigned int PylonCameraConfig::GetFrameWidth() {
        Pylon::CIntegerParameter width(nodeMap, "Width");

        return width.GetValue();
    }

    unsigned int PylonCameraConfig::GetFrameHeight() {
        Pylon::CIntegerParameter height(nodeMap, "Height");

        return height.GetValue();
    }

    unsigned int PylonCameraConfig::GetFrameOffsetX() {
        Pylon::CIntegerParameter offsetX(nodeMap, "OffsetX");

        return offsetX.GetValue();
    }

    unsigned int PylonCameraConfig::GetFrameOffsetY() {
        Pylon::CIntegerParameter offsetY(nodeMap, "OffsetY");

        return offsetY.GetValue();
    }
    double PylonCameraConfig::GetFrameRate() {
        Pylon::CFloatParameter frameRate(nodeMap, "AcquisitionFrameRate");

        return frameRate.GetValue();
    }

    std::string PylonCameraConfig::GetSerialNumber() {
        Pylon::CStringParameter serialNumber(nodeMap, "DeviceSerialNumber");

        return std::string(serialNumber.GetValue().c_str());
    }

    void PylonCameraConfig::SetFrameWidth(unsigned int newFrameWidth) {
        Pylon::CIntegerParameter frameWidth(nodeMap, "Width");

        if (!frameWidth.TrySetValue(newFrameWidth)) {
            std::cout << "Could not set frame width!" << std::endl;
        }
    }

    void PylonCameraConfig::SetFrameHeight(unsigned int newFrameHeight) {
        Pylon::CIntegerParameter frameHeight(nodeMap, "Height");

        if (!frameHeight.TrySetValue(newFrameHeight)) {
            std::cout << "Could not set frame height!" << std::endl;
        }
    }

    void PylonCameraConfig::SetFrameOffsetX(unsigned int newFrameOffsetX) {
        Pylon::CIntegerParameter frameOffsetX(nodeMap, "OffsetX");

        if (!frameOffsetX.TrySetValue(newFrameOffsetX)) {
            std::cout << "Could not set frame offset x!" << std::endl;
        }
    }

    void PylonCameraConfig::SetFrameOffsetY(unsigned int newFrameOffsetY) {
        Pylon::CIntegerParameter frameOffsetY(nodeMap, "OffsetY");

        if (!frameOffsetY.TrySetValue(newFrameOffsetY)) {
            std::cout << "Could not set frame offset y!" << std::endl;
        }
    }

    void PylonCameraConfig::SetFrameRate(double newFrameRate) {
        Pylon::CBooleanParameter frameRateEnable(nodeMap, "AcquisitionFrameRateEnable");
        Pylon::CFloatParameter frameRate(nodeMap, "AcquisitionFrameRate");

        if (frameRateEnable.TrySetValue(true)) {
            if (!frameRate.TrySetValue(newFrameRate))
                std::cout << "Could not set frame rate!" << std::endl;
        }
    }

    
    void PylonCameraConfig::SetTriggerIn(unsigned int line) {
        Pylon::CEnumParameter triggerSelector(nodeMap, "TriggerSelector");
        triggerSelector.SetValue("FrameStart");
        
        Pylon::CEnumParameter triggerMode(nodeMap, "TriggerMode");
        triggerMode.SetValue("On");

        Pylon::CEnumParameter triggerSource(nodeMap, "TriggerSource");
        triggerSource.SetValue((std::string("Line") + std::to_string(line)).c_str());

        Pylon::CEnumParameter triggerActivation(nodeMap, "TriggerActivation");
        triggerActivation.SetValue("FallingEdge");

        Pylon::CEnumParameter lineSelector(nodeMap, "LineSelector");
        lineSelector.SetValue((std::string("Line") + std::to_string(line)).c_str());

        Pylon::CEnumParameter lineMode(nodeMap, "LineMode");
        lineMode.SetValue("Input");
    }
    void PylonCameraConfig::SetTriggerOut(unsigned int line) {
        Pylon::CEnumParameter lineSelector(nodeMap, "LineSelector");
        lineSelector.SetValue((std::string("Line") + std::to_string(line)).c_str());

        Pylon::CEnumParameter lineMode(nodeMap, "LineMode");
        lineMode.SetValue("Output");

        Pylon::CEnumParameter lineSource(nodeMap, "LineSource");
        lineSource.SetValue("ExposureActive");
    }

    bool PylonCameraConfig::LoadConfiguration(std::string filepath) {
        try {
            Pylon::CFeaturePersistence::Load(filepath.c_str(), nodeMap, true);
        } catch (const Pylon::GenericException &e) {
            std::cerr << "An exception occurred." << std::endl << e.GetDescription() << std::endl;
            return false;
        }
        return true;
    }

    bool PylonCameraConfig::SaveConfiguration(std::string filepath) {
        try {
            Pylon::CFeaturePersistence::Save(filepath.c_str(), nodeMap);
        } catch (const Pylon::GenericException &e) {
            std::cerr << "An exception occurred." << std::endl << e.GetDescription() << std::endl;
            return false;
        }
        return true;
    }









    PylonCamera::PylonCamera() : running(false), armed(false) {
        cameraConfig = std::make_unique<PylonCameraConfig>();
    }
    PylonCamera::~PylonCamera() {
        Stop();
        camera.Close();
        if (camera.IsPylonDeviceAttached())
            camera.DestroyDevice();

        Pylon::PylonTerminate();
    }

    void PylonCamera::run() {
        running.store(true);

        camera.StartGrabbing(Pylon::GrabStrategy_LatestImageOnly, Pylon::GrabLoop_ProvidedByUser);

        // if (camera.WaitForFrameTriggerReady(1000, Pylon::TimeoutHandling_Return))
        //     camera.ExecuteSoftwareTrigger();
        while (running.load()) {
            // Frame Grabbing

            // Trigger line (is it triggered automatically?)

            Pylon::CGrabResultPtr currentPtrGrabResult;
            if (!camera.RetrieveResult(1000, currentPtrGrabResult, Pylon::TimeoutHandling_Return)) {
                std::this_thread::yield();
                continue;
            }
            SharedFrameHeader *result = (SharedFrameHeader *)(currentPtrGrabResult->GetBuffer()) - 1;

            // if (camera.WaitForFrameTriggerReady(1, Pylon::TimeoutHandling_Return))
            //     camera.ExecuteSoftwareTrigger();
            // else {
            //     std::this_thread::yield();
            //     continue;
            // }

            result->frameID = currentPtrGrabResult->GetImageNumber();
            result->timeStamp = currentPtrGrabResult->GetTimeStamp();
            result->numSkippedBefore =
                currentPtrGrabResult->GetNumberOfSkippedImages();  // ! Does not account for camera internal skipped images.

            latestFrame.store(result);

            // Simulate skipping images
            // Pylon::WaitObject::Sleep(500);
        }
        camera.StopGrabbing();
    }

    void PylonCamera::Init() {
        try {
            camera.Attach(Pylon::CTlFactory::GetInstance().CreateFirstDevice());
            camera.MaxNumBuffer = 30;
        } catch (const Pylon::GenericException &e) {
            // Error handling.
            std::cerr << "An exception occurred." << std::endl << e.GetDescription() << std::endl;
            throw;
        }
        camera.Open();

        cameraConfig = std::make_unique<PylonCameraConfig>();
        cameraConfig->SetNodeMap(camera.GetNodeMap());
    }
    bool PylonCamera::Init(std::string serial) {
        static Pylon::CTlFactory &tlFactory = Pylon::CTlFactory::GetInstance();
        Pylon::DeviceInfoList_t deviceList;
        tlFactory.EnumerateDevices(deviceList);

        auto match = std::find_if(deviceList.begin(), deviceList.end(), [&](const Pylon::CDeviceInfo &deviceInfo) {
            return deviceInfo.GetSerialNumber().compare(serial.c_str()) == 0;
        });
        if (match == deviceList.end())
            return false;

        try {
            camera.Attach(tlFactory.CreateDevice(*match));
            camera.MaxNumBuffer = 30;
        } catch (const Pylon::GenericException &e) {
            // Error handling.
            std::cerr << "An exception occurred." << std::endl << e.GetDescription() << std::endl;
            throw;
        }
        camera.Open();

        cameraConfig = std::make_unique<PylonCameraConfig>();
        cameraConfig->SetNodeMap(camera.GetNodeMap());

        return true;
    }
    /// Executing Arm() "fixes" the camera configuration in place.
    void PylonCamera::Arm() {
        // shrBufferFactory.PreallocateFrameBuffers((sizeof(SharedFrameHeader) + GetFrameWidth() * GetFrameHeight() /* * PixelSize*/) *
        //                                              (camera.MaxNumBuffer.GetValue() + 1),
        //                                          "vidre");

        shrBufferFactory.PreallocateFrameBuffers();

        camera.SetBufferFactory(&shrBufferFactory, Pylon::Cleanup_None);
        // camera.Close();
        armed.store(true);
    }
    void PylonCamera::Disarm() {
        // Free Shared Memory Buffers
        armed.store(false);
        // camera.Open();
    }
    void PylonCamera::Start() {
        if (!armed.load())
            return;
        latestFrame.store(nullptr);

        grabber = std::thread(&PylonCamera::run, this);
    }
    void PylonCamera::Stop() {
        running.store(false);
        if (grabber.joinable())
            grabber.join();
    }

    const SharedFrameHeader *PylonCamera::RetrieveLatestFrame() {
        return latestFrame.load();
    }
    bool PylonCamera::IsArmed() {
        return armed.load();
    }
    bool PylonCamera::IsRunning() {
        return running.load();
    }

    
    ICameraConfig *PylonCamera::GetCameraConfig() {
        return cameraConfig.get();
    }
    
    int64_t PylonCamera::GetMaxNumBuffer() {
        return camera.MaxNumBuffer.GetValue();
    }

    /// --------------------------
    /// Camera parameter access
    /// --------------------------

    void PylonCamera::SetSharedMemory(boost::interprocess::managed_shared_memory* shared_memory, std::string memory_name)
    {
        shrBufferFactory.SetSharedMemory(shared_memory, memory_name);
    }
}  // namespace vidre
