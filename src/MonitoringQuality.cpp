#include "dms/MonitoringQuality.hpp"

#include <opencv2/imgproc.hpp>

#include <cmath>

namespace dms {

MonitoringQuality::Result MonitoringQuality::assess(const cv::Mat& frameBGR,
                                                    const FaceObservation& obs) const {
    Result r;

    // No face is a presence concern, handled elsewhere; here we only judge the
    // quality of monitoring a face we DO have.
    if (!obs.faceDetected) {
        r.reliable = false;
        r.score = 0.0;
        r.reason = "no face";
        return r;
    }

    // Scene brightness (mean luma of the face region if we have it).
    cv::Rect roi = obs.face & cv::Rect(0, 0, frameBGR.cols, frameBGR.rows);
    double meanLuma = 255.0;
    if (roi.area() > 0) {
        cv::Mat gray;
        cv::cvtColor(frameBGR(roi), gray, cv::COLOR_BGR2GRAY);
        meanLuma = cv::mean(gray)[0];
    }

    const double faceFrac = static_cast<double>(obs.face.width) / frameBGR.cols;
    const bool touchesEdge = obs.face.x <= 1 || obs.face.y <= 1 ||
                             obs.face.x + obs.face.width >= frameBGR.cols - 1 ||
                             obs.face.y + obs.face.height >= frameBGR.rows - 1;

    // Rank the failure reasons; report the first (most limiting) one.
    if (meanLuma < cfg_.qualityLowLightMean) {
        r.reliable = false;
        r.reason = "low light";
    } else if (faceFrac < cfg_.qualityMinFaceWidthFraction) {
        r.reliable = false;
        r.reason = "face too small / far";
    } else if (obs.confidence < cfg_.qualityMinConfidence) {
        r.reliable = false;
        r.reason = "low detection confidence";
    } else if (!obs.hasLandmarks) {
        r.reliable = false;
        r.reason = "no landmarks";
    } else if (obs.hasHeadPose && (std::abs(obs.yaw) > cfg_.qualityMaxYawDegrees ||
                                   std::abs(obs.pitch) > cfg_.qualityMaxPitchDegrees)) {
        r.reliable = false;
        r.reason = "extreme head rotation";
    } else if (touchesEdge && faceFrac < cfg_.qualityMinFaceWidthFraction * 1.6) {
        r.reliable = false;
        r.reason = "face partially out of frame";
    }

    // A simple 0..1 score for the developer overlay.
    const double lightScore = std::min(1.0, meanLuma / 120.0);
    const double sizeScore = std::min(1.0, faceFrac / 0.25);
    r.score = r.reliable ? std::min({1.0, lightScore, sizeScore, obs.confidence + 0.4}) : 0.2;
    return r;
}

} // namespace dms
