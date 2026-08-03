#include "dms/AlertManager.hpp"

#include <algorithm>
#include <cstdio>

namespace dms {

AlertManager::State AlertManager::combine(const DrowsinessDetector::Result& drowsy,
                                          const DistractionDetector::Result& distract,
                                          double tSeconds) {
    State s;
    s.level = std::max(drowsy.level, distract.level);

    if (drowsy.level > 0) s.reasons.push_back(drowsy.message);
    if (distract.level > 0) s.reasons.push_back(distract.message);

    if (s.level >= 2) {
        // The more urgent of the two drives the headline.
        s.headline = drowsy.level >= distract.level ? drowsy.message : distract.message;
    } else if (s.level == 1) {
        s.headline = "CAUTION";
    } else {
        s.headline = "MONITORING";
    }

    // Audible alarm on level 2, rate-limited so it pulses rather than screams.
    if (cfg_.beep && s.level >= 2 && tSeconds - lastBeep_ >= 0.7) {
        std::fputc('\a', stderr);
        std::fflush(stderr);
        lastBeep_ = tSeconds;
    }
    return s;
}

} // namespace dms
