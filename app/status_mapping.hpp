#pragma once

#include <core/runtime/runtime.hpp>

#include <QColor>
#include <QString>

namespace status_mapping {
    inline QString toString(vidre::RecordingStatus status) {
        switch (status) {
            case vidre::RecordingStatus::RecordingFine:
                return "Recording";
            case vidre::RecordingStatus::RecordingDroppedFrames:
                return "Recording (Dropped)";
            case vidre::RecordingStatus::RecordingError:
                return "Recording (Error)";
            case vidre::RecordingStatus::Streaming:
                return "Streaming";
            case vidre::RecordingStatus::Stopped:
                return "Stopped";
            default:
                return "Unknown";
        }
    }

    inline QColor toColor(vidre::RecordingStatus status) {
        switch (status) {
            case vidre::RecordingStatus::RecordingFine:
                return QColor(0, 200, 0);
            case vidre::RecordingStatus::RecordingDroppedFrames:
                return QColor(220, 180, 0);
            case vidre::RecordingStatus::RecordingError:
                return QColor(220, 40, 40);
            case vidre::RecordingStatus::Streaming:
                return QColor(60, 140, 220);
            case vidre::RecordingStatus::Stopped:
                return QColor(200, 100, 0);
            default:
                return QColor(150, 150, 150);
        }
    }
}  // namespace status_mapping
