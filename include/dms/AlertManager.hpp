#pragma once

#include "dms/Config.hpp"
#include "dms/DistractionDetector.hpp"
#include "dms/DrowsinessDetector.hpp"

#include <string>
#include <vector>

namespace dms {

// Fuses the drowsiness and distraction results into a single system state and
// owns the audible alarm (rate-limited terminal bell).
class AlertManager {
public:
    struct State {
        int level = 0;                    // 0 ok, 1 warning, 2 alarm
        std::string headline;             // main banner text
        std::vector<std::string> reasons; // contributing messages
    };

    explicit AlertManager(const Config& cfg) : cfg_(cfg) {}

    State combine(const DrowsinessDetector::Result& drowsy,
                  const DistractionDetector::Result& distract,
                  double tSeconds);

private:
    Config cfg_;
    double lastBeep_ = -1e9;
};

} // namespace dms
