#pragma once

#include "dms/Config.hpp"
#include "dms/Types.hpp"

#include <opencv2/core.hpp>
#include <deque>

namespace dms {

// Optional phone/distraction-object detector. Real inference (YOLO via ONNX
// Runtime) is compiled in only when DMS_HAVE_ONNX is defined; otherwise this is
// a no-op that reports "not available" so the rest of the pipeline is unchanged.
//
// Strategy (spec section 6): run detection on ~1/N frames for performance,
// reuse the last box in between, and require several hits before confirming
// "phone in use" (temporal confirmation).
class ObjectDetector {
public:
    explicit ObjectDetector(const Config& cfg) : cfg_(cfg) {}

    bool init();                 // loads the model; false if unavailable
    bool available() const { return available_; }

    PhoneResult detect(const cv::Mat& frameBGR, double tSeconds);

private:
    Config cfg_;
    bool available_ = false;
    int frame_ = 0;

    // Temporal confirmation state.
    std::deque<bool> hits_;      // recent per-detection-cycle results
    PhoneResult last_;

#ifdef DMS_HAVE_ONNX
    // Real backend state lives in the .cpp behind the macro.
    void* impl_ = nullptr;
    PhoneResult runInference(const cv::Mat& frameBGR);
#endif
};

} // namespace dms
