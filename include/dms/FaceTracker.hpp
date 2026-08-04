#pragma once

#include "dms/Config.hpp"
#include "dms/Types.hpp"

#include <opencv2/dnn.hpp>
#include <opencv2/objdetect.hpp>
#ifdef DMS_HAVE_FACE
#include <opencv2/face.hpp>
#endif
#ifdef DMS_HAVE_DLIB
#include <dlib/image_processing/frontal_face_detector.h>
#include <dlib/image_processing/shape_predictor.h>
#endif

#include <string>

namespace dms {

// Turns a raw BGR frame into a FaceObservation.
//
// Detection backend, chosen automatically at init() by best available:
//   * dlib       - dlib HOG face detector + 68-point shape predictor (.dat).
//                  Most accurate: real EAR, MAR (yawns) and head pose.
//   * OpenCV LBF - opencv-contrib facemark model (.yaml), also 68 points.
//   * Haar       - bundled Haar cascades for face + eyes; "eyes closed" is
//                  inferred from the absence of eye detections. Always works.
class FaceTracker {
public:
    explicit FaceTracker(const Config& cfg);

    // Loads the Haar cascades (required) and, if a model path is given, the best
    // matching landmark backend. Returns false only if the Haar cascades (the
    // universal fallback) cannot be loaded.
    bool init();

    bool usingLandmarks() const { return dlibLoaded_ || facemarkLoaded_ || lmOnnxLoaded_; }
    const char* backendName() const;

    FaceObservation process(const cv::Mat& frameBGR);

private:
    Config cfg_;
    cv::CascadeClassifier faceCascade_;
    cv::CascadeClassifier eyeCascade_;

#ifdef DMS_HAVE_FACE
    cv::Ptr<cv::face::Facemark> facemark_;
#endif
    bool facemarkLoaded_ = false;

#ifdef DMS_HAVE_DLIB
    dlib::frontal_face_detector detector_;
    dlib::shape_predictor predictor_;
    FaceObservation processDlib(const cv::Mat& frameBGR);

    // Detection is the costly step, so we run it every Nth frame and reuse the
    // last box in between (the 68-point predictor still runs every frame).
    dlib::rectangle lastFace_;
    bool haveLastFace_ = false;
    double lastConfidence_ = 0.0;
    int lastCount_ = 0;
    int frameCount_ = 0;
#endif
    bool dlibLoaded_ = false;

    cv::dnn::Net lmNet_;
    bool lmOnnxLoaded_ = false;
    bool fitLandmarksOnnx(const cv::Mat& frameBGR, const cv::Rect& face, FaceObservation& obs);

    void fallbackEyes(const cv::Mat& gray, FaceObservation& obs);
    void estimateHeadPose(FaceObservation& obs, const cv::Size& frameSize) const;
};

} // namespace dms
