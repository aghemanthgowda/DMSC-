#pragma once

#include "dms/Config.hpp"
#include "dms/Types.hpp"

#include <opencv2/core.hpp>
#include <string>

namespace dms {

// Fuses head pose (temporally smoothed), approximate gaze and phone detection
// into an attention assessment. Short natural head movements are ignored;
// only sustained head/gaze-away escalates.
class DistractionDetector {
public:
    struct Result {
        bool faceVisible = true;
        DistractionLevel level = DistractionLevel::Attentive;
        double score = 0.0;               // 0..100 distraction score
        std::string headDir = "forward";  // forward|left|right|up|down|tilted|no-face
        std::string gazeDir = "unknown";
        double awaySeconds = 0.0;         // sustained head/gaze-away time
        double yaw = 0.0, pitch = 0.0, roll = 0.0;  // smoothed pose
        std::string message;
    };

    explicit DistractionDetector(const Config& cfg) : cfg_(cfg) {}

    Result update(const FaceObservation& obs, const GazeResult& gaze,
                  const PhoneResult& phone, const cv::Size& frameSize, double tSeconds);

private:
    Config cfg_;
    bool posePrimed_ = false;
    double emaYaw_ = 0.0, emaPitch_ = 0.0, emaRoll_ = 0.0;
    bool away_ = false;
    double awayStart_ = 0.0;
    bool noFace_ = false;
    double noFaceStart_ = 0.0;
};

} // namespace dms
