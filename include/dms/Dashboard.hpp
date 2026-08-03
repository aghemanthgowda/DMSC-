#pragma once

#include "dms/AlertManager.hpp"
#include "dms/DistractionDetector.hpp"
#include "dms/DrowsinessDetector.hpp"
#include "dms/EventLogger.hpp"
#include "dms/RiskEngine.hpp"
#include "dms/Types.hpp"

#include <opencv2/core.hpp>
#include <deque>
#include <string>

namespace dms {

// Renders the annotated camera view plus a professional status panel (driver
// state, risk gauge, metrics grid, event timeline) into a single canvas.
class Dashboard {
public:
    struct Frame {
        const FaceObservation* obs = nullptr;
        const DrowsinessDetector::Result* drowsy = nullptr;
        const DistractionDetector::Result* distract = nullptr;
        const GazeResult* gaze = nullptr;
        const PhoneResult* phone = nullptr;
        const RiskEngine::Result* risk = nullptr;
        const AlertManager::State* alert = nullptr;
        const std::deque<EventLogger::Event>* events = nullptr;

        double fps = 0.0;
        double inferenceMs = 0.0;
        std::string backend;
        bool landmarksActive = false;
        bool calibrating = false;
        double calibRemaining = 0.0;
        bool developer = false;
    };

    cv::Mat render(const cv::Mat& frameBGR, const Frame& f);

private:
    static constexpr int kPanelWidth = 380;
};

} // namespace dms
