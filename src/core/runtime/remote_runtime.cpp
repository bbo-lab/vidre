#include <core/runtime/remote_runtime.hpp>

#include <camera/remote_camera.hpp>
#include <core/transport/control_client.hpp>
#include <core/transport/preview_client.hpp>

namespace vidre {

    RemoteRuntimeInstance::RemoteRuntimeInstance(std::string name,
                                                 std::shared_ptr<ControlClient> control,
                                                 std::shared_ptr<PreviewClient> preview,
                                                 transport::CameraInfo info)
        : _name(std::move(name))
        , _control(std::move(control))
        , _preview(std::move(preview))
        , _info(std::move(info))
    {
        _camera = std::make_unique<RemoteCamera>(_name, _info, _control);
        refresh();
    }

    void RemoteRuntimeInstance::run()
    {
        if (_control) {
            _control->startRecording(_name);
        }
    }

    void RemoteRuntimeInstance::show()
    {
        if (_control) {
            _control->startStreaming(_name);
        }
    }

    void RemoteRuntimeInstance::stop()
    {
        if (_control) {
            _control->stopRecording(_name);
        }
    }

    std::tuple<const uint8_t*, int, int, int> RemoteRuntimeInstance::getLatestFrame()
    {
        if (!_preview) {
            return {nullptr, 0, 0, 0};
        }
        FrameEnvelope envelope;
        if (!_preview->latest(envelope) || envelope.payload.empty()) {
            return {nullptr, 0, 0, 0};
        }
        _lastEnvelope = std::move(envelope);
        _frameCache = std::move(_lastEnvelope.payload);
        return {_frameCache.data(),
                static_cast<int>(_lastEnvelope.width),
                static_cast<int>(_lastEnvelope.height),
                static_cast<int>(_lastEnvelope.frameId)};
    }

    void RemoteRuntimeInstance::setPreviewRequested(bool requested)
    {
        if (_control) {
            _control->setPreview(_name, requested);
        }
    }

    void RemoteRuntimeInstance::refresh()
    {
        if (!_control) {
            return;
        }
        _status = _control->status(_name);
        _stats = _control->stats(_name);
        if (_camera) {
            _camera->updateStatus(_status);
        }
    }

}  // namespace vidre
