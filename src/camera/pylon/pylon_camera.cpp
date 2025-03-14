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

    PylonCamera::PylonCamera() : running(false), armed(false) {}
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

    /// --------------------------
    /// Camera parameter access
    /// --------------------------

    GenApi::INodeMap &PylonCamera::GetGenAPINodeMap() {
        return camera.GetNodeMap();
    }

    unsigned int PylonCamera::GetFrameWidth() {
        Pylon::CIntegerParameter width(GetGenAPINodeMap(), "Width");

        return width.GetValue();
    }

    unsigned int PylonCamera::GetFrameHeight() {
        Pylon::CIntegerParameter height(GetGenAPINodeMap(), "Height");

        return height.GetValue();
    }

    unsigned int PylonCamera::GetFrameOffsetX() {
        Pylon::CIntegerParameter offsetX(GetGenAPINodeMap(), "OffsetX");

        return offsetX.GetValue();
    }

    unsigned int PylonCamera::GetFrameOffsetY() {
        Pylon::CIntegerParameter offsetY(GetGenAPINodeMap(), "OffsetY");

        return offsetY.GetValue();
    }

    unsigned int PylonCamera::GetMaxNumBuffer() {
        return camera.MaxNumBuffer.GetValue();
    }

    double PylonCamera::GetFrameRate() {
        Pylon::CFloatParameter frameRate(GetGenAPINodeMap(), "AcquisitionFrameRate");

        return frameRate.GetValue();
    }

    std::string PylonCamera::GetSerialNumber()
    {
        return std::string(camera.GetDeviceInfo().GetSerialNumber().c_str());
    }

    void PylonCamera::SetFrameWidth(unsigned int newFrameWidth) {
        if (!camera.IsOpen())
            return;
        Pylon::CIntegerParameter frameWidth(GetGenAPINodeMap(), "Width");

        if (!frameWidth.TrySetValue(newFrameWidth)) {
            std::cout << "Could not set frame width!" << std::endl;
        }
    }

    void PylonCamera::SetFrameHeight(unsigned int newFrameHeight) {
        if (!camera.IsOpen())
            return;
        Pylon::CIntegerParameter frameHeight(GetGenAPINodeMap(), "Height");

        if (!frameHeight.TrySetValue(newFrameHeight)) {
            std::cout << "Could not set frame height!" << std::endl;
        }
    }

    void PylonCamera::SetFrameOffsetX(unsigned int newFrameOffsetX) {
        if (!camera.IsOpen())
            return;
        Pylon::CIntegerParameter frameOffsetX(GetGenAPINodeMap(), "OffsetX");

        if (!frameOffsetX.TrySetValue(newFrameOffsetX)) {
            std::cout << "Could not set frame offset x!" << std::endl;
        }
    }

    void PylonCamera::SetFrameOffsetY(unsigned int newFrameOffsetY) {
        if (!camera.IsOpen())
            return;
        Pylon::CIntegerParameter frameOffsetY(GetGenAPINodeMap(), "OffsetY");

        if (!frameOffsetY.TrySetValue(newFrameOffsetY)) {
            std::cout << "Could not set frame offset y!" << std::endl;
        }
    }

    void PylonCamera::SetFrameRate(double newFrameRate) {
        if (!camera.IsOpen())
            return;
        GenApi::INodeMap &nodemap = GetGenAPINodeMap();
        Pylon::CBooleanParameter frameRateEnable(nodemap, "AcquisitionFrameRateEnable");
        Pylon::CFloatParameter frameRate(nodemap, "AcquisitionFrameRate");

        if (frameRateEnable.TrySetValue(true)) {
            if (!frameRate.TrySetValue(newFrameRate))
                std::cout << "Could not set frame rate!" << std::endl;
        }
    }

    
    void PylonCamera::SetTriggerIn(unsigned int line) {
        std::cout << "In: " << GetSerialNumber() << std::endl;
        GenApi::INodeMap &nodemap = GetGenAPINodeMap();

        Pylon::CEnumParameter triggerSelector(nodemap, "TriggerSelector");
        triggerSelector.SetValue("FrameStart");
        
        Pylon::CEnumParameter triggerMode(nodemap, "TriggerMode");
        triggerMode.SetValue("On");

        Pylon::CEnumParameter triggerSource(nodemap, "TriggerSource");
        triggerSource.SetValue(std::string({"Line", line}).c_str());

        Pylon::CEnumParameter triggerActivation(nodemap, "TriggerActivation");
        triggerActivation.SetValue("FallingEdge");

        Pylon::CEnumParameter lineSelector(nodemap, "LineSelector");
        lineSelector.SetValue(std::string({"Line", line}).c_str());

        Pylon::CEnumParameter lineMode(nodemap, "LineMode");
        lineMode.SetValue("Output");
    }
    void PylonCamera::SetTriggerOut(unsigned int line) {
        std::cout << "Out: " << GetSerialNumber() << std::endl;
        GenApi::INodeMap &nodemap = GetGenAPINodeMap();

        Pylon::CEnumParameter lineSelector(nodemap, "LineSelector");
        lineSelector.SetValue(std::string({"Line", line}).c_str());

        Pylon::CEnumParameter lineMode(nodemap, "LineMode");
        lineMode.SetValue("Output");

        Pylon::CEnumParameter lineSource(nodemap, "LineSource");
        lineSource.SetValue("ExposureActive");
    }

    bool PylonCamera::LoadConfiguration(std::string filepath) {
        try {
            Pylon::CFeaturePersistence::Load(filepath.c_str(), &GetGenAPINodeMap(), true);
        } catch (const Pylon::GenericException &e) {
            std::cerr << "An exception occurred." << std::endl << e.GetDescription() << std::endl;
            return false;
        }
        return true;
    }

    bool PylonCamera::SaveConfiguration(std::string filepath) {
        try {
            Pylon::CFeaturePersistence::Save(filepath.c_str(), &GetGenAPINodeMap());
        } catch (const Pylon::GenericException &e) {
            std::cerr << "An exception occurred." << std::endl << e.GetDescription() << std::endl;
            return false;
        }
        return true;
    }

    void PylonCamera::SetSharedMemory(boost::interprocess::managed_shared_memory* shared_memory, std::string memory_name)
    {
        shrBufferFactory.SetSharedMemory(shared_memory, memory_name);
    }
}  // namespace vidre
