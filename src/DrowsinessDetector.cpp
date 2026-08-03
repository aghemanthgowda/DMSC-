#include "dms/DrowsinessDetector.hpp"

namespace dms {

DrowsinessDetector::Result DrowsinessDetector::update(const FaceObservation& obs,
                                                      double tSeconds) {
    Result r;

    // Without a face we cannot judge the eyes; report the current state but do
    // not accumulate closure time (that is the distraction module's concern).
    const bool eyesClosed = obs.faceDetected && obs.eyesClosed;
    r.eyesClosed = eyesClosed;

    // --- Sustained closure + blink detection (edge-triggered) ---
    if (eyesClosed && !closed_) {
        closed_ = true;
        closureStart_ = tSeconds;
    } else if (!eyesClosed && closed_) {
        const double dur = tSeconds - closureStart_;
        if (dur >= cfg_.blinkMinSeconds && dur <= cfg_.blinkMaxSeconds) {
            ++blinkCount_;
            blinkTimes_.push_back(tSeconds);
        }
        closed_ = false;
    }
    if (closed_) {
        r.closureSeconds = tSeconds - closureStart_;
    }

    // --- PERCLOS over a rolling window ---
    if (obs.faceDetected) {
        window_.emplace_back(tSeconds, eyesClosed);
    }
    while (!window_.empty() && tSeconds - window_.front().first > cfg_.perclosWindowSeconds) {
        window_.pop_front();
    }
    if (!window_.empty()) {
        int closedCount = 0;
        for (const auto& s : window_) closedCount += s.second ? 1 : 0;
        r.perclos = static_cast<double>(closedCount) / static_cast<double>(window_.size());
    }

    // --- Blink rate (blinks in the last 60 s) ---
    while (!blinkTimes_.empty() && tSeconds - blinkTimes_.front() > 60.0) {
        blinkTimes_.pop_front();
    }
    r.blinkCount = blinkCount_;
    r.blinkRate = static_cast<double>(blinkTimes_.size());

    // --- Yawn detection (landmark path only) ---
    if (obs.mar >= 0.0) {
        const bool open = obs.mar >= cfg_.marThreshold;
        if (open && !mouthOpen_) {
            mouthOpen_ = true;
            mouthOpenStart_ = tSeconds;
            yawnCounted_ = false;
        } else if (!open && mouthOpen_) {
            mouthOpen_ = false;
        }
        if (mouthOpen_ && !yawnCounted_ &&
            tSeconds - mouthOpenStart_ >= cfg_.yawnMinSeconds) {
            ++yawnCount_;
            yawnCounted_ = true;
        }
        r.yawning = mouthOpen_ && (tSeconds - mouthOpenStart_ >= cfg_.yawnMinSeconds);
    }
    r.yawnCount = yawnCount_;

    // --- Severity ---
    if (r.closureSeconds >= cfg_.eyeClosedAlarmSeconds || r.perclos >= cfg_.perclosAlarm) {
        r.level = 2;
        r.message = r.closureSeconds >= cfg_.eyeClosedAlarmSeconds
                        ? "DROWSY - eyes closed!"
                        : "DROWSY - high PERCLOS";
    } else if (r.perclos >= cfg_.perclosWarn || r.yawning) {
        r.level = 1;
        r.message = r.yawning ? "Yawning detected" : "Fatigue building";
    } else {
        r.level = 0;
        r.message = "Alert";
    }
    return r;
}

} // namespace dms
