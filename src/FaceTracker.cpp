#include "dms/FaceTracker.hpp"

#include <opencv2/calib3d.hpp>
#include <opencv2/imgproc.hpp>

#include <algorithm>
#include <cmath>
#include <iostream>

namespace dms {

namespace {

#ifdef DMS_HAVE_FACE
double dist(const cv::Point2f& a, const cv::Point2f& b) {
    return cv::norm(a - b);
}

// Eye-Aspect-Ratio for one eye given the six landmark indices
// (outer corner, top-outer, top-inner, inner corner, bottom-inner, bottom-outer).
double eyeAspectRatio(const std::vector<cv::Point2f>& lm, const int idx[6]) {
    const double vertical = dist(lm[idx[1]], lm[idx[5]]) + dist(lm[idx[2]], lm[idx[4]]);
    const double horizontal = dist(lm[idx[0]], lm[idx[3]]);
    if (horizontal < 1e-6) return 0.0;
    return vertical / (2.0 * horizontal);
}

// Mouth-Aspect-Ratio from the inner-lip landmarks (60..67).
double mouthAspectRatio(const std::vector<cv::Point2f>& lm) {
    const double vertical =
        dist(lm[61], lm[67]) + dist(lm[62], lm[66]) + dist(lm[63], lm[65]);
    const double horizontal = dist(lm[60], lm[64]);
    if (horizontal < 1e-6) return 0.0;
    return vertical / (2.0 * horizontal);
}
#endif  // DMS_HAVE_FACE

} // namespace

FaceTracker::FaceTracker(const Config& cfg) : cfg_(cfg) {}

bool FaceTracker::init() {
    const std::string faceXml = cfg_.cascadeDir + "/haarcascade_frontalface_default.xml";
    const std::string eyeXml = cfg_.cascadeDir + "/haarcascade_eye_tree_eyeglasses.xml";

    if (!faceCascade_.load(faceXml)) {
        std::cerr << "[FaceTracker] Failed to load face cascade: " << faceXml << "\n";
        return false;
    }
    if (!eyeCascade_.load(eyeXml)) {
        std::cerr << "[FaceTracker] Failed to load eye cascade: " << eyeXml << "\n";
        return false;
    }

    if (!cfg_.facemarkModel.empty()) {
#ifdef DMS_HAVE_FACE
        try {
            facemark_ = cv::face::FacemarkLBF::create();
            facemark_->loadModel(cfg_.facemarkModel);
            facemarkLoaded_ = true;
            std::cout << "[FaceTracker] Loaded landmark model: " << cfg_.facemarkModel << "\n";
        } catch (const cv::Exception&) {
            std::cerr << "[FaceTracker] Could not load facemark model (" << cfg_.facemarkModel
                      << "); falling back to Haar eye detection.\n";
            facemarkLoaded_ = false;
        }
#else
        std::cerr << "[FaceTracker] This build has no OpenCV 'face' module; ignoring --model "
                     "and using Haar fallback mode.\n";
#endif
    }
    return true;
}

void FaceTracker::fallbackEyes(const cv::Mat& gray, FaceObservation& obs) {
    // Search only the upper ~60% of the face box, where eyes live. If the eye
    // cascade fires the eyes are open; if the face is present but no eyes are
    // found we treat the eyes as closed.
    cv::Rect upper = obs.face;
    upper.height = static_cast<int>(upper.height * 0.6);
    upper &= cv::Rect(0, 0, gray.cols, gray.rows);

    std::vector<cv::Rect> eyes;
    const cv::Mat roi = gray(upper);
    eyeCascade_.detectMultiScale(roi, eyes, 1.1, 3, 0,
                                 cv::Size(upper.width / 8, upper.height / 8));
    for (auto& e : eyes) {
        e.x += upper.x;
        e.y += upper.y;
        obs.eyes.push_back(e);
    }
    obs.eyesClosed = obs.eyes.empty();
}

void FaceTracker::estimateHeadPose(FaceObservation& obs, const cv::Size& frameSize) const {
    if (!obs.hasLandmarks) return;
    const auto& lm = obs.landmarks;

    // Generic 3D face model points (in an arbitrary mm-like scale).
    const std::vector<cv::Point3d> model = {
        {0.0, 0.0, 0.0},        // nose tip        (30)
        {0.0, -63.6, -12.5},    // chin            (8)
        {-43.3, 32.7, -26.0},   // left eye corner (36)
        {43.3, 32.7, -26.0},    // right eye corner(45)
        {-28.9, -28.9, -24.1},  // left mouth      (48)
        {28.9, -28.9, -24.1},   // right mouth     (54)
    };
    const std::vector<cv::Point2d> image = {
        lm[30], lm[8], lm[36], lm[45], lm[48], lm[54]};

    const double focal = frameSize.width;
    const cv::Point2d center(frameSize.width / 2.0, frameSize.height / 2.0);
    cv::Mat cameraMatrix = (cv::Mat_<double>(3, 3) << focal, 0, center.x, 0, focal,
                            center.y, 0, 0, 1);
    const cv::Mat distCoeffs = cv::Mat::zeros(4, 1, CV_64F);

    cv::Mat rvec, tvec;
    if (!cv::solvePnP(model, image, cameraMatrix, distCoeffs, rvec, tvec)) return;

    cv::Mat rot;
    cv::Rodrigues(rvec, rot);
    cv::Mat proj, k, r, t, rx, ry, rz, euler;
    cv::hconcat(rot, tvec, proj);
    cv::decomposeProjectionMatrix(proj, k, r, t, rx, ry, rz, euler);

    auto normalize = [](double a) {
        while (a > 90.0) a -= 180.0;
        while (a < -90.0) a += 180.0;
        return a;
    };
    obs.pitch = normalize(euler.at<double>(0));
    obs.yaw = normalize(euler.at<double>(1));
    obs.roll = normalize(euler.at<double>(2));
    obs.hasHeadPose = true;
}

FaceObservation FaceTracker::process(const cv::Mat& frameBGR) {
    FaceObservation obs;

    cv::Mat gray;
    cv::cvtColor(frameBGR, gray, cv::COLOR_BGR2GRAY);
    cv::equalizeHist(gray, gray);

    std::vector<cv::Rect> faces;
    faceCascade_.detectMultiScale(gray, faces, 1.1, 4, 0, cv::Size(80, 80));
    if (faces.empty()) {
        return obs;  // faceDetected stays false
    }

    // Track the largest face only (the driver).
    obs.face = *std::max_element(faces.begin(), faces.end(),
                                 [](const cv::Rect& a, const cv::Rect& b) {
                                     return a.area() < b.area();
                                 });
    obs.faceDetected = true;

#ifdef DMS_HAVE_FACE
    if (facemarkLoaded_) {
        std::vector<cv::Rect> one{obs.face};
        std::vector<std::vector<cv::Point2f>> shapes;
        if (facemark_->fit(gray, one, shapes) && !shapes.empty() &&
            shapes[0].size() == 68) {
            obs.landmarks = shapes[0];
            obs.hasLandmarks = true;

            static const int L[6] = {36, 37, 38, 39, 40, 41};
            static const int R[6] = {42, 43, 44, 45, 46, 47};
            const double earL = eyeAspectRatio(obs.landmarks, L);
            const double earR = eyeAspectRatio(obs.landmarks, R);
            obs.ear = 0.5 * (earL + earR);
            obs.mar = mouthAspectRatio(obs.landmarks);
            obs.eyesClosed = obs.ear < cfg_.earThreshold;

            estimateHeadPose(obs, frameBGR.size());
            return obs;
        }
    }
#endif  // DMS_HAVE_FACE

    // Fallback path (no landmarks): use the eye cascade.
    fallbackEyes(gray, obs);
    return obs;
}

} // namespace dms
