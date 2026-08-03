#include "dms/HandActivity.hpp"

#include <opencv2/imgproc.hpp>

namespace dms {

HandResult HandActivity::update(const cv::Mat& frameBGR, const FaceObservation& obs,
                                double tSeconds) {
    HandResult r;
    if (!cfg_.enableHandDetection) return r;
    r.available = true;
    if (!obs.faceDetected || obs.face.area() <= 0) {
        history_.clear();
        return r;
    }

    // Skin mask (YCrCb range) over the whole frame.
    cv::Mat ycrcb, skin;
    cv::cvtColor(frameBGR, ycrcb, cv::COLOR_BGR2YCrCb);
    cv::inRange(ycrcb, cv::Scalar(0, 133, 77), cv::Scalar(255, 173, 127), skin);
    cv::morphologyEx(skin, skin, cv::MORPH_OPEN,
                     cv::getStructuringElement(cv::MORPH_ELLIPSE, {5, 5}));
    cv::morphologyEx(skin, skin, cv::MORPH_CLOSE,
                     cv::getStructuringElement(cv::MORPH_ELLIPSE, {9, 9}));

    const cv::Rect frameRect(0, 0, frameBGR.cols, frameBGR.rows);
    cv::Rect face = obs.face & frameRect;

    // Search only the two side regions at the face's vertical range (ear/cheek
    // level) — where a hand raised to make a call / hold a phone appears. This
    // deliberately excludes below the chin (shoulders/chest are skin-coloured and
    // would false-trigger) and the face itself.
    const int sideW = std::max(1, static_cast<int>(face.width * 0.8));
    cv::Rect leftBand(face.x - sideW, face.y, sideW, face.height);
    cv::Rect rightBand(face.x + face.width, face.y, sideW, face.height);
    cv::Mat bandMask = cv::Mat::zeros(skin.size(), CV_8U);
    cv::rectangle(bandMask, leftBand & frameRect, cv::Scalar(255), cv::FILLED);
    cv::rectangle(bandMask, rightBand & frameRect, cv::Scalar(255), cv::FILLED);
    cv::rectangle(bandMask, face, cv::Scalar(0), cv::FILLED);  // exclude the face
    cv::bitwise_and(skin, bandMask, skin);

    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(skin, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    const double minArea = cfg_.handMinAreaFraction * face.area();
    const double maxArea = 1.2 * face.area();  // reject shoulder/background sheets
    bool nearRaw = false;
    double bestArea = 0.0;
    for (const auto& c : contours) {
        const double a = cv::contourArea(c);
        if (a < minArea || a > maxArea) continue;
        const cv::Rect box = cv::boundingRect(c);
        const double solidity = a / std::max(1.0, static_cast<double>(box.area()));
        if (solidity < 0.45) continue;  // prefer compact, hand-like blobs
        ++r.handsVisible;
        nearRaw = true;
        bestArea = std::max(bestArea, a);
        r.boxes.push_back(box);
    }

    // Temporal confirmation over a short window.
    history_.emplace_back(tSeconds, nearRaw);
    while (!history_.empty() && tSeconds - history_.front().first > cfg_.handConfirmSeconds) {
        history_.pop_front();
    }
    int hits = 0;
    for (const auto& h : history_) hits += h.second ? 1 : 0;
    r.handNearFace = !history_.empty() && hits * 2 >= static_cast<int>(history_.size());
    r.confidence = r.handNearFace ? std::min(1.0, bestArea / (0.5 * face.area())) : 0.0;
    return r;
}

} // namespace dms
