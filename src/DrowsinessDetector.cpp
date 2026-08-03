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
        r.earLeft = obs.earLeft;
        r.earRight = obs.earRight;
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
        if (dur >= cfg_.longBlinkSeconds) ++longBlink_;
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
    r.longBlinkCount = longBlink_;

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

    // --- Drowsiness score (0..100) + 4-level classification ---
    // The score is the strongest fatigue evidence, nudged by yawning; the level
    // adds explicit prolonged-closure / PERCLOS gates so a clear microsleep is
    // always CRITICAL regardless of the blended score.
    const double sClosure = std::min(1.0, r.closureSeconds / cfg_.eyeClosedAlarmSeconds);
    const double sPerclos = std::min(1.0, r.perclos / cfg_.perclosAlarm);
    double sev = std::max(sClosure, sPerclos);
    if (r.yawning) sev = std::min(1.0, sev + 0.15);
    r.score = 100.0 * sev;

    if (r.closureSeconds >= cfg_.eyeClosedAlarmSeconds || r.score >= 80.0) {
        r.level = DrowsyLevel::Critical;
    } else if (r.closureSeconds >= cfg_.eyeClosedDrowsySeconds ||
               r.perclos >= cfg_.perclosAlarm || r.score >= 50.0) {
        r.level = DrowsyLevel::Drowsy;
    } else if (r.perclos >= cfg_.perclosWarn || r.yawning || r.score >= 25.0) {
        r.level = DrowsyLevel::Possible;
    } else {
        r.level = DrowsyLevel::Alert;
    }
    r.message = toString(r.level);
    return r;
}

} // namespace dms
