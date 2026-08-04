#pragma once

#include "dms/Config.hpp"
#include "dms/Types.hpp"

#include <opencv2/core.hpp>
#include <opencv2/dnn.hpp>

#include <deque>
#include <vector>

namespace dms {

struct Detection {
    int classId = -1;
    float score = 0.0f;
    cv::Rect box;
};

// Real object detection via a YOLO ONNX model run with OpenCV's DNN module
// (no ONNX Runtime dependency). Detects COCO classes incl. cell phone (67).
// Phone usage is EVIDENCE-BASED only: a detected phone box, temporally confirmed,
// optionally fused with a nearby hand — never from head pose. Without a model it
// reports NO_PHONE (available=false).
class ObjectDetector {
public:
    explicit ObjectDetector(const Config& cfg) : cfg_(cfg) {}

    bool init();                       // loads cfg_.phoneModel; false if none/failed
    bool available() const { return available_; }

    // Temporal-confirmed phone result (runs YOLO ~1/N frames, reuses between).
    PhoneResult detect(const cv::Mat& frameBGR, const FaceObservation& obs,
                       const HandResult& hand, double tSeconds);

    // All detections from the most recent inference cycle (for dev overlay).
    const std::vector<Detection>& lastDetections() const { return lastDets_; }

private:
    Config cfg_;
    cv::dnn::Net net_;
    bool available_ = false;
    int inputSize_ = 640;
    int frame_ = 0;

    std::vector<Detection> lastDets_;
    std::deque<bool> phoneHits_;
    cv::Rect lastPhoneBox_;
    double lastPhoneConf_ = 0.0;

    std::vector<Detection> runYolo(const cv::Mat& frameBGR);
};

} // namespace dms
