#pragma once

#include "dms/Config.hpp"
#include "dms/Types.hpp"

#include <opencv2/core.hpp>

namespace dms {

// Approximate visual-attention (gaze) estimator. Locates the darkest blob (the
// pupil) inside each eye's landmark box and reports its offset from the eye
// centre. This is a coarse cue — NOT medically/scientifically exact gaze — and
// is only produced when landmarks are present and the eyes are open.
class GazeEstimator {
public:
    explicit GazeEstimator(const Config& cfg) : cfg_(cfg) {}

    GazeResult estimate(const cv::Mat& frameBGR, const FaceObservation& obs);

private:
    Config cfg_;
    double emaDx_ = 0.0;
    double emaDy_ = 0.0;
    bool primed_ = false;

    // Pupil offset within one eye (six landmark indices), or false if unreliable.
    bool eyeOffset(const cv::Mat& gray, const FaceObservation& obs, const int idx[6],
                   double& dx, double& dy) const;
};

} // namespace dms
