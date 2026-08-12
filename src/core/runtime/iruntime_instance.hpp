#pragma once

#include <cstdint>
#include <string>
#include <tuple>

#include <camera/camera.hpp>

namespace vidre {
    enum class RecordingStatus {
        Stopped = 0,
        Streaming = 1,
        RecordingFine = 2,
        RecordingDroppedFrames = 3,
        RecordingError = 4
    };

    class IRuntimeInstance {
    public:
        virtual ~IRuntimeInstance() = default;

        virtual void run() = 0;
        virtual void show() = 0;
        virtual void stop() = 0;

        virtual bool isRunning() = 0;
        virtual bool checkCamera() = 0;

        virtual RecordingStatus getRecordingStatus() = 0;
        virtual ICamera* getCamera() = 0;

        virtual unsigned int getFrameWidth() = 0;
        virtual unsigned int getFrameHeight() = 0;
        virtual unsigned int getFrameBytes() = 0;

        virtual std::tuple<const uint8_t*, int, int, int> getLatestFrame() = 0;

        virtual uint64_t getBitsPerPixel() = 0;
        virtual int getDroppedFrames() = 0;
        virtual int getSavedFramesNumber() = 0;
        virtual std::string getOutputFileName() = 0;
        virtual int64_t getSentFrames() { return 0; }

        virtual void setPreviewRequested(bool requested) = 0;

        virtual void refresh() {}
    };
}  // namespace vidre
