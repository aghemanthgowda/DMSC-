#pragma once

#include <opencv2/core.hpp>
#include <string>
#include <vector>

namespace dms {

// A single-frame result produced by the FaceTracker. Everything downstream
// (drowsiness, distraction, alerting) consumes this struct so the rest of the
// pipeline never needs to know whether we ran the landmark model or the plain
// Haar-cascade fallback.
struct FaceObservation {
    bool faceDetected = false;
    cv::Rect face;                        // face box in full-frame coordinates
    std::vector<cv::Rect> eyes;           // eye boxes from the Haar fallback path

    bool hasLandmarks = false;
    std::vector<cv::Point2f> landmarks;   // 68 points when hasLandmarks == true

    bool eyesClosed = false;              // best per-frame estimate
    double ear = -1.0;                    // averaged eye-aspect-ratio (>=0 with landmarks)
    double mar = -1.0;                    // mouth-aspect-ratio (>=0 with landmarks)

    bool hasHeadPose = false;
    double yaw = 0.0;                     // degrees, + = turned to driver's right
    double pitch = 0.0;                   // degrees, + = looking up
    double roll = 0.0;                    // degrees
};

} // namespace dms
