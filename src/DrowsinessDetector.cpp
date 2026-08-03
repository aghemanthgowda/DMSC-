#include "dms/DrowsinessDetector.hpp"

#include <algorithm>
#include <numeric>

namespace dms {

double DrowsinessDetector::smoothMar(double rawMar) {
    marSmooth_.push_back(rawMar);
    while (marSmooth_.size() > static_cast<size_t>(cfg_.marSmoothingFrames)) {
        marSmooth_.pop_front();
    }
    return std::accumulate(marSmooth_.begin(), marSmooth_.end(), 0.0) /
           static_cast<double>(marSmooth_.size());
}

// Smooth the raw EAR, learn the driver's own "eyes open" baseline (the rolling
// max of the smoothed EAR), and flag eyes-closed relative to that baseline. This
// adapts to different faces, glasses and camera distances far better than a
// single fixed threshold.
bool DrowsinessDetector::decideEyesClosed(double rawEar, double t, double& smoothedOut,
                                          double& thresholdOut) {
    earSmooth_.push_back(rawEar);
    while (earSmooth_.size() > static_cast<size_t>(cfg_.earSmoothingFrames)) {
        earSmooth_.pop_front();
    }
    const double smoothed =
        std::accumulate(earSmooth_.begin(), earSmooth_.end(), 0.0) /
        static_cast<double>(earSmooth_.size());
    smoothedOut = smoothed;

    // Rolling "open" baseline = max smoothed EAR over the recent window.
    earBaseline_.emplace_back(t, smoothed);
    while (!earBaseline_.empty() &&
           t - earBaseline_.front().first > cfg_.earBaselineWindowSeconds) {
        earBaseline_.pop_front();
    }
    double baseline = 0.0;
    for (const auto& e : earBaseline_) baseline = std::max(baseline, e.second);

    // Threshold = ratio of the open baseline, clamped to a sane band. Until we
    // have a confident baseline, fall back to the fixed threshold.
    double threshold = cfg_.earThreshold;
    if (baseline > 0.15) {
        threshold = std::clamp(baseline * cfg_.earCloseRatio, 0.15, 0.30);
    }
    thresholdOut = threshold;
    return smoothed < threshold;
}

DrowsinessDetector::Result DrowsinessDetector::update(const FaceObservation& obs,
                                                      double tSeconds) {
    Result r;
    if (startTime_ < 0.0) startTime_ = tSeconds;

    // Decide eyes-closed. With landmarks we use the smoothed, self-calibrating
    // EAR; without them we fall back to the tracker's per-frame estimate.
    bool eyesClosed;
    if (obs.faceDetected && obs.ear >= 0.0) {
        double smoothed = 0.0, threshold = 0.0;
        eyesClosed = decideEyesClosed(obs.ear, tSeconds, smoothed, threshold);
        r.ear = smoothed;
        r.earThreshold = threshold;
        r.calibrating = (tSeconds - startTime_) < cfg_.earBaselineWindowSeconds;
    } else {
        eyesClosed = obs.faceDetected && obs.eyesClosed;
    }
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
        const double mar = smoothMar(obs.mar);
        r.mar = mar;
        const bool open = mar >= cfg_.marThreshold;
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
