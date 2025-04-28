#include "vidre/pylon_camera.hpp"

namespace vidre {
PylonCameraConfig::PylonCameraConfig() {
    nodeMap = GenApi::CNodeMapFactory::CreateEmptyNodeMap();
}
    
void PylonCameraConfig::SetNodeMap(GenApi::INodeMap &newNodeMap) {
    nodeMap = &newNodeMap;
}

// Generalized parameter getters and setters
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

// Specific parameter getters and setters
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
    SetEnum("TriggerActivation", "RisingEdge");
    SetEnum("LineSelector", (std::string("Line") + std::to_string(line)).c_str());
    SetEnum("LineMode", "Input");
}
void PylonCameraConfig::SetTriggerOut(int64_t line) {
    SetEnum("LineSelector", (std::string("Line") + std::to_string(line)).c_str());
    SetEnum("LineMode", "Output");
    SetEnum("LineSource", "FrameTrigger");
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









PylonCamera::PylonCamera() : armed(false), initialized(false) {}
PylonCamera::~PylonCamera() {
    Stop();
    camera->Close();
    camera->DeregisterImageEventHandler(this);
    if (camera->IsPylonDeviceAttached())
        camera->DestroyDevice();

    Pylon::PylonTerminate();
}

void PylonCamera::OnImageGrabbed( Pylon::CInstantCamera& camera, const Pylon::CGrabResultPtr& ptrGrabResult ) {
    // Assumption space reserved before data for Header.
    SharedFrameHeader *result = (SharedFrameHeader *)(ptrGrabResult->GetBuffer()) - 1;
    result->frameID = ptrGrabResult->GetImageNumber();
    result->timeStamp = ptrGrabResult->GetTimeStamp();
    result->numSkippedBefore = ptrGrabResult->GetNumberOfSkippedImages();  // ! Does not account for camera internal skipped images.

    //if (!ptrGrabResult->GrabSucceeded()) {
    //    std::cerr << "Error: " << ptrGrabResult->GetErrorCode() << ": " << ptrGrabResult->GetErrorDescription() << "\n";
    //    return;
    //}
    // Announce newest frame availability
    latestFrame = result;
}
void PylonCamera::OnCameraDeviceRemoved( Pylon::CInstantCamera& camera ) {
    initialized = false;
    camera.StopGrabbing();
    Disarm();
    std::cout << "Camera with serial number " << serialNumber << " was disconnected!" << std::endl;
    // This needs to be wrapped up in some signaling so that outside classes can act upon CameraDeviceRemoval.
    // E.g. having a list of functors that are called. Same for other events. Generalize it to other modules aswell.

    camera.DetachDevice();
    camera.DestroyDevice();
}

void PylonCamera::_Init() {
    camera->Open();
    GenApi::INodeMap &nodeMap = camera->GetNodeMap();

    camera->MaxNumBuffer = 30;
    camera->MaxNumQueuedBuffer = 30;
    camera->MaxNumGrabResults = 1;

    camera->RegisterImageEventHandler(this, Pylon::ERegistrationMode::RegistrationMode_Append, Pylon::ECleanup::Cleanup_None);
    camera->RegisterConfiguration(this, Pylon::ERegistrationMode::RegistrationMode_Append, Pylon::ECleanup::Cleanup_None);

    cameraConfig.SetNodeMap(nodeMap);
    serialNumber = camera->GetDeviceInfo().GetSerialNumber();

    //camera->GrabLoopThreadPriorityOverride = true;
    //camera->GrabLoopThreadPriority = 24;

    initialized = true;
}

void PylonCamera::Init() {
    camera.release();
    camera = std::make_unique<Pylon::CInstantCamera>();
    try {
        camera->Attach(Pylon::CTlFactory::GetInstance().CreateFirstDevice());
    } catch (const Pylon::GenericException &e) {
        // Error handling.
        std::cerr << "An exception occurred." << std::endl << e.GetDescription() << std::endl;
        throw;
    }
    _Init();
}
bool PylonCamera::Init(std::string serial) {
    camera.release();
    camera = std::make_unique<Pylon::CInstantCamera>();
    Pylon::DeviceInfoList_t deviceList;
    Pylon::CTlFactory& tlFactory = Pylon::CTlFactory::GetInstance();
    tlFactory.EnumerateDevices(deviceList);

    auto match = std::find_if(deviceList.begin(), deviceList.end(), [&](const Pylon::CDeviceInfo &deviceInfo) {
        return deviceInfo.GetSerialNumber().compare(serial.c_str()) == 0;
    });
    if (match == deviceList.end())
        return false;
    
    try {
        camera->Attach(tlFactory.CreateDevice(*match));
    } catch (const Pylon::GenericException &e) {
        // Error handling.
        std::cerr << "An exception occurred." << std::endl << e.GetDescription() << std::endl;
        throw e;
    }
    

    _Init();

    return true;
}
/// Executing Arm() "fixes" the camera configuration in place.
void PylonCamera::Arm() {
    if (!initialized) Init(std::string(serialNumber.c_str()));
    
    shrBufferFactory.PreallocateFrameBuffers();

    camera->SetBufferFactory(&shrBufferFactory, Pylon::Cleanup_None);
    //camera->Close();
    armed.store(true);
}
void PylonCamera::Disarm() {
    if (!initialized) Init(std::string(serialNumber.c_str()));

    armed.store(false);
    //if (!camera->IsCameraDeviceRemoved())
    //    camera->Open();
}
void PylonCamera::Start() {
    if (!armed
        || camera->IsCameraDeviceRemoved()) {
        return;
    }

    if (!initialized) {
        if (!Init(std::string(serialNumber.c_str())))
            return;
    }

    latestFrame = nullptr;

    camera->StartGrabbing(Pylon::EGrabStrategy::GrabStrategy_LatestImageOnly, Pylon::EGrabLoop::GrabLoop_ProvidedByInstantCamera);
}
void PylonCamera::Stop() {
    if (!initialized) Init(std::string(serialNumber.c_str()));
    camera->StopGrabbing();
}

const SharedFrameHeader *PylonCamera::RetrieveLatestFrame() {
    return latestFrame;
}
bool PylonCamera::IsArmed() {
    if (!initialized || camera->IsCameraDeviceRemoved()) return false;
    return armed.load();
}
bool PylonCamera::IsRunning() {
    if (!initialized || camera->IsCameraDeviceRemoved()) return false;
    return camera->IsGrabbing();
}


ICameraConfig &PylonCamera::GetCameraConfig() {
    return cameraConfig;
}

int64_t PylonCamera::GetMaxNumBuffer() {
    return camera->MaxNumBuffer.GetValue();
}

void PylonCamera::SetSharedMemory(boost::interprocess::managed_shared_memory* shared_memory, std::string memory_name) {
    shrBufferFactory.SetSharedMemory(shared_memory, memory_name);
}
}  // namespace vidre
