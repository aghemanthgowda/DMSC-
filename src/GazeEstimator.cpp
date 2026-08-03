#include "dms/GazeEstimator.hpp"

#include <opencv2/imgproc.hpp>

#include <algorithm>

namespace dms {

bool GazeEstimator::eyeOffset(const cv::Mat& gray, const FaceObservation& obs,
                              const int idx[6], double& dx, double& dy) const {
    // Bounding box of the six eye landmarks, padded slightly.
    cv::Point2f lo = obs.landmarks[idx[0]], hi = obs.landmarks[idx[0]];
    for (int i = 1; i < 6; ++i) {
        lo.x = std::min(lo.x, obs.landmarks[idx[i]].x);
        lo.y = std::min(lo.y, obs.landmarks[idx[i]].y);
        hi.x = std::max(hi.x, obs.landmarks[idx[i]].x);
        hi.y = std::max(hi.y, obs.landmarks[idx[i]].y);
    }
    const int padX = static_cast<int>((hi.x - lo.x) * 0.15) + 1;
    const int padY = static_cast<int>((hi.y - lo.y) * 0.35) + 1;
    cv::Rect roi(cv::Point(static_cast<int>(lo.x) - padX, static_cast<int>(lo.y) - padY),
                 cv::Point(static_cast<int>(hi.x) + padX, static_cast<int>(hi.y) + padY));
    roi &= cv::Rect(0, 0, gray.cols, gray.rows);
    if (roi.width < 8 || roi.height < 5) return false;

    cv::Mat eye = gray(roi);
    cv::Mat blurred;
    cv::GaussianBlur(eye, blurred, cv::Size(5, 5), 0);
    double minVal, maxVal;
    cv::Point minLoc, maxLoc;
    cv::minMaxLoc(blurred, &minVal, &maxVal, &minLoc, &maxLoc);  // darkest = pupil

    // Offset of the pupil from the eye-box centre, normalized to [-1, 1].
    dx = (static_cast<double>(minLoc.x) / roi.width - 0.5) * 2.0;
    dy = (static_cast<double>(minLoc.y) / roi.height - 0.5) * 2.0;
    return true;
}

GazeResult GazeEstimator::estimate(const cv::Mat& frameBGR, const FaceObservation& obs) {
    GazeResult g;
    // Need open eyes and landmarks; a closed/searching eye gives meaningless blobs.
    if (!obs.hasLandmarks || obs.landmarks.size() != 68 || obs.eyesClosed) {
        primed_ = false;
        return g;
    }

    cv::Mat gray;
    cv::cvtColor(frameBGR, gray, cv::COLOR_BGR2GRAY);

    static const int L[6] = {36, 37, 38, 39, 40, 41};
    static const int R[6] = {42, 43, 44, 45, 46, 47};
    double lx, ly, rx, ry;
    const bool okL = eyeOffset(gray, obs, L, lx, ly);
    const bool okR = eyeOffset(gray, obs, R, rx, ry);
    if (!okL && !okR) return g;

    double dx = 0.0, dy = 0.0;
    int n = 0;
    if (okL) { dx += lx; dy += ly; ++n; }
    if (okR) { dx += rx; dy += ry; ++n; }
    dx /= n;
    dy /= n;

    // Exponential smoothing for stability.
    if (!primed_) {
        emaDx_ = dx;
        emaDy_ = dy;
        primed_ = true;
    } else {
        emaDx_ = 0.6 * emaDx_ + 0.4 * dx;
        emaDy_ = 0.6 * emaDy_ + 0.4 * dy;
    }

    g.valid = true;
    g.dx = emaDx_;
    g.dy = emaDy_;
    const double t = cfg_.gazeOffThreshold;
    if (emaDx_ > t) g.direction = "right";
    else if (emaDx_ < -t) g.direction = "left";
    else if (emaDy_ < -t) g.direction = "up";
    else if (emaDy_ > t) g.direction = "down";
    else g.direction = "forward";
    return g;
}

} // namespace dms
