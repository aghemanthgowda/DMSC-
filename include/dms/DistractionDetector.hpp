#pragma once

#include "dms/Config.hpp"
#include "dms/Types.hpp"

#include <opencv2/core.hpp>
#include <string>

namespace dms {

// Decides whether the driver's attention is on the road, using head pose when
// landmarks are available and a face-position heuristic otherwise.
class DistractionDetector {
public:
    struct Result {
        bool faceVisible = true;
        bool lookingAway = false;
        double awaySeconds = 0.0;      // sustained no-face / look-away time
        std::string direction = "center";  // center|left|right|down|no-face
        int level = 0;                 // 0 ok, 1 warning, 2 alarm
        std::string message;
    };

    explicit DistractionDetector(const Config& cfg) : cfg_(cfg) {}

    Result update(const FaceObservation& obs, const cv::Size& frameSize, double tSeconds);

private:
    Config cfg_;
    bool away_ = false;
    double awayStart_ = 0.0;
};

} // namespace dms
