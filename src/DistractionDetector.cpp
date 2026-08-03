#include "dms/DistractionDetector.hpp"

#include <cmath>

namespace dms {

DistractionDetector::Result DistractionDetector::update(const FaceObservation& obs,
                                                        const cv::Size& frameSize,
                                                        double tSeconds) {
    Result r;
    r.faceVisible = obs.faceDetected;

    std::string direction = "center";

    if (!obs.faceDetected) {
        direction = "no-face";
    } else if (obs.hasHeadPose) {
        // Landmark path: use estimated head orientation.
        if (obs.yaw > cfg_.yawDistractDegrees) {
            direction = "right";
        } else if (obs.yaw < -cfg_.yawDistractDegrees) {
            direction = "left";
        } else if (obs.pitch < -cfg_.pitchDownDegrees) {
            direction = "down";
        }
    } else {
        // Fallback: how far the face centre sits from the frame centre.
        const double faceCx = obs.face.x + obs.face.width * 0.5;
        const double dx = (faceCx - frameSize.width * 0.5) / frameSize.width;
        if (dx > cfg_.offCenterFraction) {
            direction = "right";
        } else if (dx < -cfg_.offCenterFraction) {
            direction = "left";
        }
    }

    r.direction = direction;
    r.lookingAway = direction != "center";

    // Sustain timer: how long we have been continuously away.
    if (r.lookingAway) {
        if (!away_) {
            away_ = true;
            awayStart_ = tSeconds;
        }
        r.awaySeconds = tSeconds - awayStart_;
    } else {
        away_ = false;
    }

    // Severity.
    if (direction == "no-face") {
        r.level = r.awaySeconds >= cfg_.noFaceAlarmSeconds ? 2 : 1;
        r.message = r.level == 2 ? "DRIVER NOT DETECTED" : "Face lost";
    } else if (r.lookingAway) {
        r.level = r.awaySeconds >= cfg_.lookAwayAlarmSeconds ? 2 : 1;
        const std::string where = direction == "down" ? "looking down" : ("looking " + direction);
        r.message = (r.level == 2 ? "EYES OFF ROAD - " : "Distraction - ") + where;
    } else {
        r.level = 0;
        r.message = "Eyes on road";
    }
    return r;
}

} // namespace dms
