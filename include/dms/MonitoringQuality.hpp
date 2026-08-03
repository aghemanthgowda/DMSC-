#pragma once

#include "dms/Config.hpp"
#include "dms/Types.hpp"

#include <opencv2/core.hpp>
#include <string>

namespace dms {

// Assesses whether the system can RELIABLY monitor the driver this frame.
// AIS-184-oriented safety behaviour: when monitoring is unreliable the decision
// engine must NOT assert drowsiness — it reports "monitoring quality low" so a
// dark frame, a tiny/edge face or an extreme head turn never becomes a false
// "DRIVER DROWSY".
class MonitoringQuality {
public:
    struct Result {
        bool reliable = true;
        double score = 1.0;        // 0..1 rough quality
        std::string reason;        // human-readable cause when not reliable
    };

    explicit MonitoringQuality(const Config& cfg) : cfg_(cfg) {}

    Result assess(const cv::Mat& frameBGR, const FaceObservation& obs) const;

private:
    Config cfg_;
};

} // namespace dms
