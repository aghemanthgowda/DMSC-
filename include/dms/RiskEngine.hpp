#pragma once

#include "dms/Config.hpp"
#include "dms/DistractionDetector.hpp"
#include "dms/DrowsinessDetector.hpp"
#include "dms/Types.hpp"

#include <string>
#include <vector>

namespace dms {

// Central fusion. Combines the drowsiness, distraction and phone signals into a
// single 0..100 risk score and a stable DriverState, applying a temporal state
// machine (hysteresis + persistence) so the state cannot flicker frame-to-frame.
class RiskEngine {
public:
    struct Result {
        double score = 0.0;                // smoothed 0..100 overall risk
        DriverState state = DriverState::Safe;
        int alertLevel = 0;                // 0 none, 1 visual, 2 audio, 3 critical
        std::vector<std::string> reasons;  // contributing signal messages
        // Raw contributions (for developer mode).
        double drowsyContribution = 0.0;
        double distractionContribution = 0.0;
        double phoneContribution = 0.0;
        double yawnContribution = 0.0;
    };

    explicit RiskEngine(const Config& cfg) : cfg_(cfg) {}

    Result update(const DrowsinessDetector::Result& drowsy,
                  const DistractionDetector::Result& distract, const PhoneResult& phone,
                  bool driverPresent, double tSeconds);

private:
    Config cfg_;
    double emaRisk_ = 0.0;
    bool primed_ = false;

    DriverState current_ = DriverState::Safe;
    DriverState pending_ = DriverState::Safe;
    double pendingSince_ = 0.0;
};

} // namespace dms
