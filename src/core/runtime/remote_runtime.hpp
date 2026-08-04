#pragma once

#include <memory>
#include <string>
#include <vector>

#include <camera/remote_camera.hpp>
#include <core/runtime/iruntime_instance.hpp>
#include <core/transport/wire_protocol.hpp>

namespace vidre {

    class ControlClient;
    class PreviewClient;

    class RemoteRuntimeInstance : public IRuntimeInstance {
    public:
        RemoteRuntimeInstance(std::string name,
                              std::shared_ptr<ControlClient> control,
                              std::shared_ptr<PreviewClient> preview,
                              transport::CameraInfo info);
        ~RemoteRuntimeInstance() override = default;

        void run() override;
        void show() override;
        void stop() override;

        bool isRunning() override { return _status.running; }
        bool checkCamera() override { return true; }

        RecordingStatus getRecordingStatus() override {
            return static_cast<RecordingStatus>(_status.recordingStatus);
        }
        ICamera* getCamera() override { return _camera.get(); }

        unsigned int getFrameWidth() override { return _info.width; }
        unsigned int getFrameHeight() override { return _info.height; }
        unsigned int getFrameBytes() override { return _info.frameBytes; }

        std::tuple<const uint8_t*, int, int, int> getLatestFrame() override;

        uint64_t getBitsPerPixel() override { return _info.bitsPerPixel; }
        int getDroppedFrames() override { return static_cast<int>(_stats.droppedFrames); }
        int getSavedFramesNumber() override { return static_cast<int>(_stats.savedFrames); }
        std::string getOutputFileName() override { return ""; }

        void setPreviewRequested(bool requested) override;
        void refresh() override;

        const std::string& name() const { return _name; }

    private:
        std::string _name;
        std::shared_ptr<ControlClient> _control;
        std::shared_ptr<PreviewClient> _preview;
        std::unique_ptr<RemoteCamera> _camera;
        transport::CameraInfo _info;
        transport::CameraStatus _status;
        transport::RecordingStats _stats;
        std::vector<uint8_t> _frameCache;
        FrameEnvelope _lastEnvelope;
    };

}  // namespace vidre
