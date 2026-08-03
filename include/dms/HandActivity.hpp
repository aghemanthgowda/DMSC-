#pragma once

#include "dms/Config.hpp"
#include "dms/Types.hpp"

#include <opencv2/core.hpp>
#include <deque>

namespace dms {

// EXTENDED DMS feature (approximate, no ML dependency). Detects skin-coloured
// blobs in a band around the driver's face — a hand raised to the ear/cheek/
// mouth — and temporally confirms "hand near face". Combined by the caller with
// a phone-like head posture to infer possible phone use. Deliberately coarse;
// the accurate path is the ONNX YOLO ObjectDetector. Never gates core DDAW.
class HandActivity {
public:
    explicit HandActivity(const Config& cfg) : cfg_(cfg) {}

    HandResult update(const cv::Mat& frameBGR, const FaceObservation& obs, double tSeconds);

private:
    Config cfg_;
    std::deque<std::pair<double, bool>> history_;  // (time, handNearFace raw)
};

} // namespace dms
