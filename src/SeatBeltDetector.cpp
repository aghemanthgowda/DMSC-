#include "dms/SeatBeltDetector.hpp"

namespace dms {

SeatBeltResult SeatBeltDetector::update(const cv::Mat& frameBGR, const FaceObservation& obs) {
    SeatBeltResult r;
    if (!obs.faceDetected || obs.face.area() <= 0) {
        r.state = SeatBeltState::Unknown;
        return r;
    }

    // Torso ROI: below the face, ~2.2x face width centered on the face, extending
    // down toward the bottom of the frame — where a shoulder belt crosses.
    const cv::Rect frameRect(0, 0, frameBGR.cols, frameBGR.rows);
    const int w = static_cast<int>(obs.face.width * 2.2);
    const int cx = obs.face.x + obs.face.width / 2;
    const int top = obs.face.y + static_cast<int>(obs.face.height * 1.1);
    cv::Rect roi(cx - w / 2, top, w, frameBGR.rows - top);
    r.torsoRoi = roi & frameRect;

    // No seat-belt model available -> honest UNKNOWN. A custom detector would set
    // classIdSeatbelt >= 0 and populate this with a real result.
    if (cfg_.classIdSeatbelt < 0) {
        r.available = false;
        r.state = SeatBeltState::Unknown;
        r.confidence = 0.0;
        return r;
    }

    // (Reserved) With a seat-belt model, run it inside r.torsoRoi and set
    // Detected/Fastened/NotDetected + confidence here.
    r.available = true;
    r.state = SeatBeltState::Unknown;
    return r;
}

} // namespace dms
