#include "dms/ObjectDetector.hpp"

#include <opencv2/imgproc.hpp>

#include <algorithm>
#include <iostream>
#include <numeric>
#include <sys/stat.h>

namespace dms {

namespace {
bool fileExists(const std::string& p) {
    struct stat st{};
    return !p.empty() && stat(p.c_str(), &st) == 0;
}
} // namespace

bool ObjectDetector::init() {
    if (cfg_.phoneModel.empty() || !fileExists(cfg_.phoneModel)) {
        available_ = false;
        if (!cfg_.phoneModel.empty())
            std::cerr << "[ObjectDetector] model not found: " << cfg_.phoneModel << "\n";
        return false;
    }
    try {
        net_ = cv::dnn::readNetFromONNX(cfg_.phoneModel);
        net_.setPreferableBackend(cv::dnn::DNN_BACKEND_OPENCV);
        net_.setPreferableTarget(cv::dnn::DNN_TARGET_CPU);
        available_ = true;
        std::cout << "[ObjectDetector] Loaded YOLO model (OpenCV DNN): " << cfg_.phoneModel << "\n";
    } catch (const cv::Exception& e) {
        std::cerr << "[ObjectDetector] Failed to load model: " << e.what() << "\n";
        available_ = false;
    }
    return available_;
}

// YOLOv8 ONNX inference with letterbox pre-processing and manual output parsing.
std::vector<Detection> ObjectDetector::runYolo(const cv::Mat& frameBGR) {
    std::vector<Detection> dets;
    const int S = inputSize_;

    // Letterbox to SxS keeping aspect ratio.
    const double r = std::min(static_cast<double>(S) / frameBGR.cols,
                              static_cast<double>(S) / frameBGR.rows);
    const int newW = static_cast<int>(std::round(frameBGR.cols * r));
    const int newH = static_cast<int>(std::round(frameBGR.rows * r));
    const int padX = (S - newW) / 2, padY = (S - newH) / 2;
    cv::Mat resized, canvas(S, S, frameBGR.type(), cv::Scalar(114, 114, 114));
    cv::resize(frameBGR, resized, cv::Size(newW, newH));
    resized.copyTo(canvas(cv::Rect(padX, padY, newW, newH)));

    cv::Mat blob;
    cv::dnn::blobFromImage(canvas, blob, 1.0 / 255.0, cv::Size(S, S), cv::Scalar(), true, false);
    net_.setInput(blob);
    cv::Mat out = net_.forward();  // YOLOv8: [1, 84, 8400]

    // Normalize to a [num, dim] matrix. Supports both YOLOv5 (dim = 4 + 1 obj +
    // numClasses, e.g. 85 for COCO) and YOLOv8 (dim = 4 + numClasses, e.g. 84).
    if (out.dims == 3) out = out.reshape(1, out.size[1]);
    if (out.rows < out.cols) cv::transpose(out, out);
    const int dim = out.cols;
    const bool hasObjectness = (dim == 85);   // v5 COCO; v8 COCO = 84
    const int classStart = hasObjectness ? 5 : 4;
    const int numClasses = dim - classStart;
    if (numClasses <= 0) return dets;

    std::vector<int> ids;
    std::vector<float> scores;
    std::vector<cv::Rect> boxes;
    for (int i = 0; i < out.rows; ++i) {
        const float* d = out.ptr<float>(i);
        const float objn = hasObjectness ? d[4] : 1.0f;
        if (objn < 0.10f) continue;
        int bestId = 0;
        float best = 0.0f;
        for (int c = 0; c < numClasses; ++c) {
            if (d[classStart + c] > best) { best = d[classStart + c]; bestId = c; }
        }
        best *= objn;
        if (best < cfg_.yoloConfidence) continue;
        // Box is cx,cy,w,h in letterboxed pixels -> map back to frame.
        const float cx = d[0], cy = d[1], w = d[2], h = d[3];
        const int x = static_cast<int>((cx - w / 2 - padX) / r);
        const int y = static_cast<int>((cy - h / 2 - padY) / r);
        const int bw = static_cast<int>(w / r), bh = static_cast<int>(h / r);
        ids.push_back(bestId);
        scores.push_back(best);
        boxes.emplace_back(x, y, bw, bh);
    }

    std::vector<int> keep;
    cv::dnn::NMSBoxes(boxes, scores, static_cast<float>(cfg_.yoloConfidence),
                      static_cast<float>(cfg_.nmsThreshold), keep);
    for (int idx : keep) {
        dets.push_back({ids[idx], scores[idx], boxes[idx] & cv::Rect(0, 0, frameBGR.cols, frameBGR.rows)});
    }
    return dets;
}

PhoneResult ObjectDetector::detect(const cv::Mat& frameBGR, const FaceObservation& obs,
                                   const HandResult& hand, double tSeconds) {
    (void)tSeconds;
    PhoneResult r;
    if (!available_) return r;  // NO_PHONE, available=false
    r.available = true;

    // Run inference every Nth frame; reuse detections in between.
    if (frame_++ % std::max(1, cfg_.phoneDetectEveryNFrames) == 0) {
        lastDets_ = runYolo(frameBGR);
    }

    // Best phone detection this cycle.
    bool phoneNow = false;
    cv::Rect phoneBox;
    double phoneConf = 0.0;
    for (const auto& d : lastDets_) {
        if (d.classId == cfg_.classIdPhone && d.score > phoneConf) {
            phoneNow = true;
            phoneConf = d.score;
            phoneBox = d.box;
        }
    }

    // Temporal confirmation over the last few detection cycles.
    phoneHits_.push_back(phoneNow);
    while (phoneHits_.size() > static_cast<size_t>(std::max(1, cfg_.phoneConfirmFrames)))
        phoneHits_.pop_front();
    const int hits = std::accumulate(phoneHits_.begin(), phoneHits_.end(), 0,
                                     [](int a, bool b) { return a + (b ? 1 : 0); });
    if (phoneNow) { lastPhoneBox_ = phoneBox; lastPhoneConf_ = phoneConf; }

    // Hand fusion: is the phone box near a detected hand? (normalized by face width)
    bool heldByHand = false;
    if (phoneNow && obs.faceDetected && obs.face.width > 1 && !hand.boxes.empty()) {
        const cv::Point2f pc(phoneBox.x + phoneBox.width * 0.5f, phoneBox.y + phoneBox.height * 0.5f);
        for (const auto& b : hand.boxes) {
            const cv::Point2f hc(b.x + b.width * 0.5f, b.y + b.height * 0.5f);
            if (cv::norm(hc - pc) / obs.face.width < cfg_.handPhoneDistanceFraction) {
                heldByHand = true;
                break;
            }
        }
    }

    // State machine.
    if (hits >= cfg_.phoneConfirmFrames) {
        r.state = heldByHand ? PhoneState::UsageConfirmed : PhoneState::Detected;
    } else if (hits > 0) {
        r.state = PhoneState::Possible;
    } else {
        r.state = PhoneState::NoPhone;
    }
    r.phonePresent = (r.state == PhoneState::Detected || r.state == PhoneState::UsageConfirmed);
    r.heldByHand = heldByHand;
    r.confidence = lastPhoneConf_;
    r.box = lastPhoneBox_;
    return r;
}

} // namespace dms
