#include "dms/ObjectDetector.hpp"

#include <iostream>
#include <numeric>

namespace dms {

bool ObjectDetector::init() {
#ifdef DMS_HAVE_ONNX
    // Phase 2 wires the ONNX Runtime session here (see ObjectDetectorOnnx.cpp).
    available_ = onnxInit();
#else
    available_ = false;
    if (!cfg_.phoneModel.empty()) {
        std::cout << "[ObjectDetector] Phone model set but this build has no ONNX "
                     "support; phone detection disabled.\n";
    }
#endif
    return available_;
}

PhoneResult ObjectDetector::detect(const cv::Mat& frameBGR, double tSeconds) {
    (void)tSeconds;
    (void)frameBGR;
    PhoneResult r;
    if (!available_) return r;  // available stays false -> phonePresent false
    r.available = true;

#ifdef DMS_HAVE_ONNX
    // Run inference only every Nth frame; reuse the last box otherwise.
    const bool doRun = (frame_++ % std::max(1, cfg_.phoneDetectEveryNFrames)) == 0;
    if (doRun) {
        const PhoneResult inst = runInference(frameBGR);
        hits_.push_back(inst.confidence >= cfg_.phoneConfidenceThreshold);
        while (hits_.size() > static_cast<size_t>(cfg_.phoneConfirmFrames)) hits_.pop_front();
        last_ = inst;
    }
    // Temporal confirmation: present only if the last few cycles agree.
    const int hitCount = std::accumulate(hits_.begin(), hits_.end(), 0,
                                         [](int a, bool b) { return a + (b ? 1 : 0); });
    r = last_;
    r.available = true;
    r.phonePresent = hits_.size() >= static_cast<size_t>(cfg_.phoneConfirmFrames) &&
                     hitCount >= cfg_.phoneConfirmFrames;
    (void)frameBGR;
#endif
    return r;
}

} // namespace dms
