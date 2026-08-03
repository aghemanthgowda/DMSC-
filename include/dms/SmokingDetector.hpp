#pragma once

#include "dms/Config.hpp"
#include "dms/Types.hpp"

#include <deque>

namespace dms {

// Smoking detection (Extended DMS). Correct multi-cue logic: a cigarette object
// near the mouth + a hand near the mouth, temporally confirmed. HONEST BEHAVIOUR:
// COCO YOLO has no cigarette class, so without a custom cigarette model
// (classIdCigarette >= 0 and a cigarette detection) this stays Unknown — it is
// never inferred from head/hand movement alone.
class SmokingDetector {
public:
    explicit SmokingDetector(const Config& cfg) : cfg_(cfg) {}

    // `cigarette` is a detection from a custom model (available=false if none).
    // `mouth` is the mouth center (from landmarks); `hand` the hand activity.
    SmokingResult update(const PhoneResult& cigarette, const cv::Point2f& mouth,
                         double faceWidth, const HandResult& hand, double tSeconds);

private:
    Config cfg_;
    std::deque<std::pair<double, bool>> history_;  // (time, cue-present)
};

} // namespace dms
