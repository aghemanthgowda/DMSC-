#include "dms/SmokingDetector.hpp"

#include <opencv2/core.hpp>

namespace dms {

SmokingResult SmokingDetector::update(const PhoneResult& cigarette, const cv::Point2f& mouth,
                                      double faceWidth, const HandResult& hand, double tSeconds) {
    SmokingResult r;

    // No cigarette-capable model -> honest UNKNOWN. Never fake from movement.
    if (cfg_.classIdCigarette < 0 || !cigarette.available) {
        r.available = false;
        r.state = SmokingState::Unknown;
        return r;
    }
    r.available = true;

    // Multi-cue: cigarette detected AND near the mouth AND a hand near the mouth.
    bool cue = false;
    if (cigarette.phonePresent && faceWidth > 1.0) {
        const cv::Point2f cigC(cigarette.box.x + cigarette.box.width * 0.5f,
                               cigarette.box.y + cigarette.box.height * 0.5f);
        const double cigMouth = cv::norm(cigC - mouth) / faceWidth;
        bool handNearMouth = false;
        for (const auto& b : hand.boxes) {
            const cv::Point2f hc(b.x + b.width * 0.5f, b.y + b.height * 0.5f);
            if (cv::norm(hc - mouth) / faceWidth < cfg_.handMouthDistanceFraction) {
                handNearMouth = true;
                break;
            }
        }
        cue = cigMouth < cfg_.handMouthDistanceFraction && handNearMouth;
    }

    history_.emplace_back(tSeconds, cue);
    while (!history_.empty() && tSeconds - history_.front().first > 1.5) history_.pop_front();
    int hits = 0;
    for (const auto& h : history_) hits += h.second ? 1 : 0;

    if (hits >= 3 && hits * 2 >= static_cast<int>(history_.size())) {
        r.state = SmokingState::Confirmed;
        r.confidence = cigarette.confidence;
    } else if (cue) {
        r.state = SmokingState::Possible;
        r.confidence = cigarette.confidence * 0.5;
    } else {
        r.state = SmokingState::NoSmoking;
    }
    r.box = cigarette.box;
    return r;
}

} // namespace dms
