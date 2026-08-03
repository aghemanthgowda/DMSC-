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
        int yawnCount = 0;
        bool yawning = false;
        int level = 0;                // 0 ok, 1 warning, 2 alarm
        std::string message;
    };

    explicit DrowsinessDetector(const Config& cfg) : cfg_(cfg) {}

    Result update(const FaceObservation& obs, double tSeconds);

private:
    Config cfg_;

    bool closed_ = false;
    double closureStart_ = 0.0;

    int blinkCount_ = 0;
    int yawnCount_ = 0;
    bool mouthOpen_ = false;
    double mouthOpenStart_ = 0.0;
    bool yawnCounted_ = false;

    std::deque<std::pair<double, bool>> window_;  // (time, eyesClosed) for PERCLOS
    std::deque<double> blinkTimes_;               // blink timestamps for the rate
};

} // namespace dms
