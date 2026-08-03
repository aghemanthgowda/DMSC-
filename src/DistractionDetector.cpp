#include "dms/DistractionDetector.hpp"

#include <algorithm>
#include <cmath>

namespace dms {

DistractionDetector::Result DistractionDetector::update(const FaceObservation& obs,
                                                        const GazeResult& gaze,
                                                        const PhoneResult& phone,
                                                        const cv::Size& frameSize,
                                                        double tSeconds) {
    Result r;
    r.faceVisible = obs.faceDetected;
    r.gazeDir = gaze.valid ? gaze.direction : "unknown";

    // ---- No face: presence signal (grace handled by the risk/presence layer) ----
    if (!obs.faceDetected) {
        if (!noFace_) { noFace_ = true; noFaceStart_ = tSeconds; }
        const double dt = tSeconds - noFaceStart_;
        r.headDir = "no-face";
        r.awaySeconds = dt;
        r.score = std::min(100.0, 40.0 + 40.0 * std::clamp(dt / cfg_.noFaceAlarmSeconds, 0.0, 1.0));
        r.level = dt >= cfg_.noFaceAlarmSeconds ? DistractionLevel::Highly
                                                : DistractionLevel::Brief;
        r.message = dt >= cfg_.noFaceAlarmSeconds ? "DRIVER NOT DETECTED" : "Face lost";
        away_ = false;
        return r;
    }
    noFace_ = false;

    // ---- Head pose (temporally smoothed) ----
    const double a = cfg_.headPoseSmoothing;
    if (obs.hasHeadPose) {
        if (!posePrimed_) {
            emaYaw_ = obs.yaw; emaPitch_ = obs.pitch; emaRoll_ = obs.roll;
            posePrimed_ = true;
        } else {
            emaYaw_ = (1 - a) * emaYaw_ + a * obs.yaw;
            emaPitch_ = (1 - a) * emaPitch_ + a * obs.pitch;
            emaRoll_ = (1 - a) * emaRoll_ + a * obs.roll;
        }
        if (emaYaw_ > cfg_.headAwayYawDegrees) r.headDir = "right";
        else if (emaYaw_ < -cfg_.headAwayYawDegrees) r.headDir = "left";
        else if (emaPitch_ < -cfg_.headDownPitchDegrees) r.headDir = "down";
        else if (emaPitch_ > cfg_.headDownPitchDegrees) r.headDir = "up";
        else if (std::abs(emaRoll_) > cfg_.headAwayYawDegrees) r.headDir = "tilted";
        else r.headDir = "forward";
    } else {
        // Fallback: horizontal face-centre offset.
        const double cx = obs.face.x + obs.face.width * 0.5;
        const double dx = (cx - frameSize.width * 0.5) / frameSize.width;
        if (dx > cfg_.offCenterFraction) r.headDir = "right";
        else if (dx < -cfg_.offCenterFraction) r.headDir = "left";
        else r.headDir = "forward";
    }
    r.yaw = emaYaw_; r.pitch = emaPitch_; r.roll = emaRoll_;

    // ---- Sustain timer for head/gaze-away ----
    const bool headAway = r.headDir != "forward";
    const bool gazeAway = gaze.valid && gaze.direction != "forward";
    if (headAway || gazeAway) {
        if (!away_) { away_ = true; awayStart_ = tSeconds; }
        r.awaySeconds = tSeconds - awayStart_;
    } else {
        away_ = false;
    }

    // ---- Distraction score (0..100) ----
    double s = 0.0;
    const double sustain = std::clamp(r.awaySeconds / cfg_.headAwayDurationSeconds, 0.0, 1.0);
    if (headAway) s += 30.0 + 40.0 * sustain;                 // grows as it persists
    if (gazeAway) s += 15.0;
    if (r.headDir == "down" && gaze.valid && gaze.direction == "down") s += 15.0;  // combo
    if (phone.phonePresent) s += 25.0;                        // phone-in-view raises it
    r.score = std::clamp(s, 0.0, 100.0);

    // ---- Level ----
    if (r.score >= 70.0) r.level = DistractionLevel::Highly;
    else if (r.score >= 45.0) r.level = DistractionLevel::Distracted;
    else if (r.score >= 20.0) r.level = DistractionLevel::Brief;
    else r.level = DistractionLevel::Attentive;

    // ---- Message ----
    if (r.level == DistractionLevel::Attentive) {
        r.message = "Eyes on road";
    } else if (phone.phonePresent) {
        r.message = "Phone / looking away";
    } else if (headAway) {
        r.message = "Looking " + r.headDir;
    } else {
        r.message = "Gaze off road";
    }
    return r;
}

} // namespace dms
