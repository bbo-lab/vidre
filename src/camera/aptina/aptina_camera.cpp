#include "aptina_camera.hpp"

namespace vidre {
    AptinaCamera::AptinaCamera() : running(false), armed(false) {}
    AptinaCamera::~AptinaCamera() {
    }

    void AptinaCamera::run() {
    }

    void AptinaCamera::Init() {
    }
    bool AptinaCamera::Init(std::string serial) {
        return true;
    }
    void AptinaCamera::Arm() {
    }
    void AptinaCamera::Disarm() {
    }
    void AptinaCamera::Start() {
    }
    void AptinaCamera::Stop() {
    }

    const SharedFrameHeader* AptinaCamera::RetrieveLatestFrame() {
        const SharedFrameHeader* a;
        return a;
    }
    bool AptinaCamera::IsArmed() {
        return armed.load();
    }
    bool AptinaCamera::IsRunning() {
        return grabber.joinable();
    }

    unsigned int AptinaCamera::GetFrameWidth() {
       return 0;
    }

    unsigned int AptinaCamera::GetFrameHeight() {
        return 0;
    }

    unsigned int AptinaCamera::GetFrameOffsetX() {
        return 0;
    }

    unsigned int AptinaCamera::GetFrameOffsetY() {
        return 0;
    }

    unsigned int AptinaCamera::GetMaxNumBuffer() {
        return 0;
    }

    double AptinaCamera::GetFrameRate() {
        return 0;
    }

    std::string AptinaCamera::GetSerialNumber()
    {
        return "";
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

    
    void AptinaCamera::SetTriggerIn(unsigned int line) {
    }
    void AptinaCamera::SetTriggerOut(unsigned int line) {
    }

    bool AptinaCamera::LoadConfiguration(std::string filepath) {
        return true;
    }

    bool AptinaCamera::SaveConfiguration(std::string filepath) {
        return true;
    }

    void AptinaCamera::SetSharedMemory(boost::interprocess::managed_shared_memory* sharedMemory, std::vector<void *>* ptrs, std::string memory_name)
    {
    }
}  // namespace vidre
