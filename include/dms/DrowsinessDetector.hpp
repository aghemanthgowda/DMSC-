#pragma once

#include "dms/Config.hpp"
#include "dms/Types.hpp"

#include <deque>
#include <string>

namespace dms {

// Consumes per-frame observations over time and derives fatigue signals:
// sustained eye-closure (microsleep), PERCLOS, blink rate and yawns.
class DrowsinessDetector {
public:
    struct Result {
        bool eyesClosed = false;
        double closureSeconds = 0.0;  // current uninterrupted eye-closure
        double perclos = 0.0;         // fraction of the window with eyes closed [0..1]
        int blinkCount = 0;
        double blinkRate = 0.0;       // blinks per minute (rolling 60s)
        int longBlinkCount = 0;       // blinks longer than cfg.longBlinkSeconds
        int yawnCount = 0;
        bool yawning = false;

        DrowsyLevel level = DrowsyLevel::Alert;  // ALERT/POSSIBLE/DROWSY/CRITICAL
        double score = 0.0;           // 0..100 drowsiness score for risk fusion
        std::string message;

        double ear = -1.0;            // smoothed EAR (>=0 with landmarks)
        double earLeft = -1.0;
        double earRight = -1.0;
        double earThreshold = 0.0;    // adaptive eyes-closed threshold in use
        double mar = -1.0;            // smoothed MAR (>=0 with landmarks)
        bool calibrating = false;     // still learning the open-eye baseline
    };

    explicit DrowsinessDetector(const Config& cfg) : cfg_(cfg) {}

    Result update(const FaceObservation& obs, double tSeconds);

private:
    Config cfg_;

    bool closed_ = false;
    double closureStart_ = 0.0;

    int blinkCount_ = 0;
    int longBlink_ = 0;
    int yawnCount_ = 0;
    bool mouthOpen_ = false;
    double mouthOpenStart_ = 0.0;
    bool yawnCounted_ = false;

    std::deque<std::pair<double, bool>> window_;  // (time, eyesClosed) for PERCLOS
    std::deque<double> blinkTimes_;               // blink timestamps for the rate

    std::deque<double> earSmooth_;                // moving-average buffer for EAR
    std::deque<double> marSmooth_;                // moving-average buffer for MAR
    std::deque<std::pair<double, double>> earBaseline_;  // (time, smoothedEAR) for open-baseline
    double startTime_ = -1.0;                     // first observation time (for calibration)

    // Smoothed value + adaptive eyes-closed decision from a raw landmark EAR.
    bool decideEyesClosed(double rawEar, double t, double& smoothedOut, double& thresholdOut);
    double smoothMar(double rawMar);
};

} // namespace dms
