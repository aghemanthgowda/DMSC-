#pragma once

#include "dms/Config.hpp"
#include "dms/Types.hpp"

#include <opencv2/core.hpp>

namespace dms {

// Seat-belt monitoring (Extended DMS). Locates the driver torso ROI (below the
// face) where a belt would cross the chest. HONEST BEHAVIOUR: because COCO YOLO
// has no seat-belt class, this returns SeatBeltState::Unknown until a custom
// seat-belt detection/segmentation model is plugged in (classIdSeatbelt >= 0).
// It never reports "not worn" merely because the belt is not visible.
class SeatBeltDetector {
public:
    explicit SeatBeltDetector(const Config& cfg) : cfg_(cfg) {}

    // `beltBox` is an optional belt detection from a custom model (empty if none).
    SeatBeltResult update(const cv::Mat& frameBGR, const FaceObservation& obs);

private:
    Config cfg_;
};

} // namespace dms
