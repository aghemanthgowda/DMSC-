#pragma once

#include "dms/Config.hpp"
#include "dms/RiskEngine.hpp"

#include <string>
#include <vector>

namespace dms {

// Turns the fused risk state into user-facing alerts. Owns the audible alarm
// with cooldown + escalation so it pulses rather than fires every frame, and
// prioritizes critical events over minor ones.
class AlertManager {
public:
    struct State {
        int level = 0;                     // 0 none, 1 visual, 2 audio, 3 critical
        std::string headline;              // short banner text
        std::string message;               // spoken/notification-style message
        std::vector<std::string> reasons;
    };

    explicit AlertManager(const Config& cfg) : cfg_(cfg) {}

    State update(const RiskEngine::Result& risk, double tSeconds);

private:
    Config cfg_;
    double lastBeep_ = -1e9;
};

} // namespace dms
