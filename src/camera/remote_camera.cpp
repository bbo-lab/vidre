#include <camera/remote_camera.hpp>

#include <core/transport/control_client.hpp>

namespace vidre {

    RemoteCamera::RemoteCamera(std::string name, transport::CameraInfo info, std::shared_ptr<ControlClient> control)
        : _name(std::move(name))
        , _info(std::move(info))
        , _control(std::move(control))
    {
    }

    void RemoteCamera::SetExposure(double newExposure)
    {
        if (_control) {
            _control->setExposure(_name, newExposure);
        }
        _info.exposure = newExposure;
    }

    void RemoteCamera::SetGain(double newGain)
    {
        if (_control) {
            _control->setGain(_name, newGain);
        }
        _info.gain = newGain;
    }

}  // namespace vidre
