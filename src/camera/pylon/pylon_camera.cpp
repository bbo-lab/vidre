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
        nodeMap = &newNodeMap;
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
    void PylonCameraConfig::SetFloat(std::string parameter, double value) {
        Pylon::CFloatParameter(nodeMap, parameter.c_str()).SetValue(value);
    }
    void PylonCameraConfig::SetString(std::string parameter, std::string value) {
        Pylon::CStringParameter(nodeMap, parameter.c_str()).SetValue(value.c_str());
    }
    void PylonCameraConfig::SetEnum(std::string parameter, std::string value) {
        Pylon::CEnumParameter(nodeMap, parameter.c_str()).SetValue(value.c_str());
    }
    bool PylonCameraConfig::GetBool(std::string parameter) {
        return Pylon::CBooleanParameter(nodeMap, parameter.c_str()).GetValue();
    }
    int64_t PylonCameraConfig::GetInt(std::string parameter) {
        return Pylon::CIntegerParameter(nodeMap, parameter.c_str()).GetValue();
    }
    double PylonCameraConfig::GetFloat(std::string parameter) {
        return Pylon::CFloatParameter(nodeMap, parameter.c_str()).GetValue();
    }
    std::string PylonCameraConfig::GetString(std::string parameter) {
        return Pylon::CStringParameter(nodeMap, parameter.c_str()).GetValue().c_str();
    }
    std::string PylonCameraConfig::GetEnum(std::string parameter) {
        return Pylon::CEnumParameter(nodeMap, parameter.c_str()).GetValue().c_str();
    }
    

    int64_t PylonCameraConfig::GetFrameWidth() {
        return GetInt("Width");
    }

    int64_t PylonCameraConfig::GetFrameHeight() {
        return GetInt("Height");
    }

    int64_t PylonCameraConfig::GetFrameOffsetX() {
        return GetInt("OffsetX");
    }

    int64_t PylonCameraConfig::GetFrameOffsetY() {
        return GetInt("OffsetY");
    }
    double PylonCameraConfig::GetFrameRate() {
        return GetFloat("AcquisitionFrameRate");
    }

    std::string PylonCameraConfig::GetSerialNumber() {
        return GetString("DeviceSerialNumber");
    }

    void PylonCameraConfig::SetFrameWidth(int64_t newFrameWidth) {
        SetInt("Width", newFrameWidth);
    }

    void PylonCameraConfig::SetFrameHeight(int64_t newFrameHeight) {
        SetInt("Width", newFrameHeight);
    }

    void PylonCameraConfig::SetFrameOffsetX(int64_t newFrameOffsetX) {
        SetInt("OffsetX", newFrameOffsetX);
    }

    void PylonCameraConfig::SetFrameOffsetY(int64_t newFrameOffsetY) {
        SetInt("OffsetY", newFrameOffsetY);
    }

    void PylonCameraConfig::SetFrameRate(double newFrameRate) {
        SetBool("AcquisitionFrameRateEnable", true);
        SetFloat("AcquisitionFrameRate", newFrameRate);
    }

    
    void PylonCameraConfig::SetTriggerIn(int64_t line) {
        SetEnum("TriggerSelector", "FrameStart");
        SetEnum("TriggerMode", "On");
        SetEnum("TriggerSource", (std::string("Line") + std::to_string(line)).c_str());
        SetEnum("TriggerActivation", "FallingEdge");
        SetEnum("LineSelector", (std::string("Line") + std::to_string(line)).c_str());
        SetEnum("LineMode", "Input");
    }
    void PylonCameraConfig::SetTriggerOut(int64_t line) {
        SetEnum("LineSelector", (std::string("Line") + std::to_string(line)).c_str());
        SetEnum("LineMode", "Output");
        SetEnum("LineSource", "ExposureActive");
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

            // Potential delegation function to consumers, notifying about a new frame. Maybe have some general post-processing chain.

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
