#pragma once

#include "dms/AlertManager.hpp"
#include "dms/DistractionDetector.hpp"
#include "dms/DrowsinessDetector.hpp"
#include "dms/Types.hpp"

#include <opencv2/core.hpp>

namespace dms {

// Renders the annotated camera view plus a status panel into a single canvas
// that main() shows in a window.
class Dashboard {
public:
    cv::Mat render(const cv::Mat& frameBGR,
                   const FaceObservation& obs,
                   const DrowsinessDetector::Result& drowsy,
                   const DistractionDetector::Result& distract,
                   const AlertManager::State& alert,
                   double fps,
                   bool usingLandmarks);

private:
    static constexpr int kPanelWidth = 340;
};

} // namespace dms
