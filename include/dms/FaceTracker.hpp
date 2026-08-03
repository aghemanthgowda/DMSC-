#pragma once

#include "dms/Config.hpp"
#include "dms/Types.hpp"

#include <opencv2/face.hpp>
#include <opencv2/objdetect.hpp>

#include <string>

namespace dms {

// Turns a raw BGR frame into a FaceObservation.
//
// Two detection paths, chosen automatically at init():
//   * Landmark path  - if an LBF facemark model is provided, we fit 68 facial
//                      landmarks and compute a real Eye-Aspect-Ratio (EAR),
//                      Mouth-Aspect-Ratio (MAR) and head pose (solvePnP).
//   * Fallback path  - otherwise we use Haar cascades for the face and eyes and
//                      infer "eyes closed" from the absence of eye detections.
class FaceTracker {
public:
    explicit FaceTracker(const Config& cfg);

    // Loads the required Haar cascades and, if facemarkModel is non-empty and
    // valid, the optional landmark model. Returns false only when the mandatory
    // cascades cannot be loaded.
    bool init();

    bool usingLandmarks() const { return facemarkLoaded_; }

    FaceObservation process(const cv::Mat& frameBGR);

private:
    Config cfg_;
    cv::CascadeClassifier faceCascade_;
    cv::CascadeClassifier eyeCascade_;
    cv::Ptr<cv::face::Facemark> facemark_;
    bool facemarkLoaded_ = false;

    void fallbackEyes(const cv::Mat& gray, FaceObservation& obs);
    void estimateHeadPose(FaceObservation& obs, const cv::Size& frameSize) const;
};

} // namespace dms
