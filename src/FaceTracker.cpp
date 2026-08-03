#include "dms/FaceTracker.hpp"

#include <opencv2/calib3d.hpp>
#include <opencv2/imgproc.hpp>

#ifdef DMS_HAVE_DLIB
#include <dlib/opencv.h>
#include <dlib/image_processing.h>
#endif

#include <algorithm>
#include <cmath>
#include <iostream>

namespace dms {

namespace {

bool endsWith(const std::string& s, const std::string& suffix) {
    return s.size() >= suffix.size() &&
           s.compare(s.size() - suffix.size(), suffix.size(), suffix) == 0;
}

#if defined(DMS_HAVE_FACE) || defined(DMS_HAVE_DLIB)
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

// Fill EAR / MAR / eyesClosed from a populated 68-point landmark set.
void fillLandmarkMetrics(FaceObservation& obs, const Config& cfg) {
    static const int L[6] = {36, 37, 38, 39, 40, 41};
    static const int R[6] = {42, 43, 44, 45, 46, 47};
    const double earL = eyeAspectRatio(obs.landmarks, L);
    const double earR = eyeAspectRatio(obs.landmarks, R);
    obs.ear = 0.5 * (earL + earR);
    obs.mar = mouthAspectRatio(obs.landmarks);
    obs.eyesClosed = obs.ear < cfg.earThreshold;
}
#endif  // DMS_HAVE_FACE || DMS_HAVE_DLIB

} // namespace

FaceTracker::FaceTracker(const Config& cfg) : cfg_(cfg) {}

const char* FaceTracker::backendName() const {
    if (dlibLoaded_) return "dlib 68-point landmarks";
    if (facemarkLoaded_) return "OpenCV 68-point landmarks";
    return "Haar cascade";
}

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
        const bool isDat = endsWith(cfg_.facemarkModel, ".dat");

#ifdef DMS_HAVE_DLIB
        if (isDat) {
            try {
                detector_ = dlib::get_frontal_face_detector();
                dlib::deserialize(cfg_.facemarkModel) >> predictor_;
                dlibLoaded_ = true;
                std::cout << "[FaceTracker] Loaded dlib 68-point model: " << cfg_.facemarkModel
                          << "\n";
            } catch (const std::exception& e) {
                std::cerr << "[FaceTracker] Could not load dlib model (" << e.what()
                          << "); falling back to Haar.\n";
                dlibLoaded_ = false;
            }
        }
#endif
#ifdef DMS_HAVE_FACE
        if (!dlibLoaded_ && !isDat) {
            try {
                facemark_ = cv::face::FacemarkLBF::create();
                facemark_->loadModel(cfg_.facemarkModel);
                facemarkLoaded_ = true;
                std::cout << "[FaceTracker] Loaded OpenCV landmark model: " << cfg_.facemarkModel
                          << "\n";
            } catch (const cv::Exception&) {
                std::cerr << "[FaceTracker] Could not load OpenCV facemark model ("
                          << cfg_.facemarkModel << "); falling back to Haar.\n";
                facemarkLoaded_ = false;
            }
        }
#endif
        if (!dlibLoaded_ && !facemarkLoaded_) {
            if (isDat) {
                std::cerr << "[FaceTracker] '" << cfg_.facemarkModel
                          << "' is a dlib model but this build has no dlib support; "
                             "using Haar fallback.\n";
            } else {
                std::cerr << "[FaceTracker] '" << cfg_.facemarkModel
                          << "' needs the OpenCV 'face' module which is missing; "
                             "using Haar fallback.\n";
            }
        }
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

#ifdef DMS_HAVE_DLIB
FaceObservation FaceTracker::processDlib(const cv::Mat& frameBGR) {
    FaceObservation obs;

    // HOG detection is the costly step, so run it on a half-size image and scale
    // the box back up. The 68-point predictor then runs on the full-res frame.
    const double scale = 0.5;
    cv::Mat small;
    cv::resize(frameBGR, small, cv::Size(), scale, scale, cv::INTER_LINEAR);

    dlib::cv_image<dlib::bgr_pixel> dsmall(small);
    std::vector<dlib::rectangle> dets = detector_(dsmall);
    if (dets.empty()) return obs;

    const dlib::rectangle best = *std::max_element(
        dets.begin(), dets.end(),
        [](const dlib::rectangle& a, const dlib::rectangle& b) { return a.area() < b.area(); });

    const dlib::rectangle full(
        static_cast<long>(best.left() / scale), static_cast<long>(best.top() / scale),
        static_cast<long>(best.right() / scale), static_cast<long>(best.bottom() / scale));

    obs.face = cv::Rect(cv::Point(static_cast<int>(full.left()), static_cast<int>(full.top())),
                        cv::Point(static_cast<int>(full.right()) + 1,
                                  static_cast<int>(full.bottom()) + 1));
    obs.face &= cv::Rect(0, 0, frameBGR.cols, frameBGR.rows);
    obs.faceDetected = true;

    dlib::cv_image<dlib::bgr_pixel> dfull(frameBGR);
    const dlib::full_object_detection shape = predictor_(dfull, full);
    if (shape.num_parts() == 68) {
        obs.landmarks.reserve(68);
        for (unsigned i = 0; i < 68; ++i) {
            obs.landmarks.emplace_back(static_cast<float>(shape.part(i).x()),
                                       static_cast<float>(shape.part(i).y()));
        }
        obs.hasLandmarks = true;
        fillLandmarkMetrics(obs, cfg_);
        estimateHeadPose(obs, frameBGR.size());
    }
    return obs;
}
#endif  // DMS_HAVE_DLIB

FaceObservation FaceTracker::process(const cv::Mat& frameBGR) {
#ifdef DMS_HAVE_DLIB
    if (dlibLoaded_) return processDlib(frameBGR);
#endif

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
            fillLandmarkMetrics(obs, cfg_);
            estimateHeadPose(obs, frameBGR.size());
            return obs;
        }
    }
#endif

    // Fallback path (no landmarks): use the eye cascade.
    fallbackEyes(gray, obs);
    return obs;
}

} // namespace dms
