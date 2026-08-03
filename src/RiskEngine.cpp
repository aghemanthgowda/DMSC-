#include "dms/RiskEngine.hpp"

#include <algorithm>

namespace dms {

namespace {
// Severity ranking used by the hysteresis logic (higher = more urgent).
int severity(DriverState s) {
    switch (s) {
        case DriverState::Safe: return 0;
        case DriverState::AttentionRequired: return 1;
        case DriverState::Drowsy: return 2;
        case DriverState::Distracted: return 2;
        case DriverState::PhoneUsage: return 2;
        case DriverState::HighRisk: return 3;
        case DriverState::Critical: return 4;
    }
    return 0;
}
} // namespace

RiskEngine::Result RiskEngine::update(const DrowsinessDetector::Result& drowsy,
                                      const DistractionDetector::Result& distract,
                                      const PhoneResult& phone, bool driverPresent,
                                      bool monitoringReliable, double tSeconds) {
    Result r;
    r.monitoringReliable = monitoringReliable;

    // When monitoring is unreliable we must NOT assert drowsiness/yawn evidence
    // (AIS-184-oriented: distinguish "monitoring unreliable" from "driver drowsy").
    const double phoneScore = phone.phonePresent ? 100.0 : 0.0;
    const double drowsyScore = monitoringReliable ? drowsy.score : 0.0;
    const double yawnScore = (monitoringReliable && drowsy.yawning) ? 100.0 : 0.0;

    // Weighted blend (weights are configurable and normalized by their sum).
    r.drowsyContribution = cfg_.wDrowsiness * drowsyScore;
    r.distractionContribution = cfg_.wDistraction * distract.score;
    r.phoneContribution = cfg_.wPhone * phoneScore;
    r.yawnContribution = cfg_.wYawn * yawnScore;
    const double wSum = cfg_.wDrowsiness + cfg_.wDistraction + cfg_.wPhone + cfg_.wYawn;
    double risk = (r.drowsyContribution + r.distractionContribution + r.phoneContribution +
                   r.yawnContribution) /
                  (wSum > 1e-6 ? wSum : 1.0);

    // A strong, sustained SINGLE signal must not be diluted away by the blend
    // (e.g. a driver looking away with everything else normal is still risky).
    risk = std::max(risk, drowsyScore * 0.85);
    risk = std::max(risk, distract.score * 0.60);
    risk = std::max(risk, phoneScore * 0.90);

    // A confirmed absent driver is inherently high risk.
    if (!driverPresent) risk = std::max(risk, cfg_.riskHighThreshold);
    risk = std::clamp(risk, 0.0, 100.0);

    // Smooth the score for a steady gauge.
    if (!primed_) { emaRisk_ = risk; primed_ = true; }
    else emaRisk_ = 0.7 * emaRisk_ + 0.3 * risk;
    r.score = emaRisk_;

    // Candidate state from thresholds + dominant signal.
    DriverState candidate;
    if (!driverPresent && distract.awaySeconds >= cfg_.noFaceAlarmSeconds) {
        candidate = DriverState::HighRisk;
    } else if (r.score >= cfg_.riskCriticalThreshold) {
        candidate = DriverState::Critical;
    } else if (r.score >= cfg_.riskHighThreshold) {
        candidate = DriverState::HighRisk;
    } else if (r.score >= cfg_.riskWarnThreshold) {
        if (phone.phonePresent && phoneScore >= drowsyScore) {
            candidate = DriverState::PhoneUsage;
        } else if (drowsyScore >= distract.score) {
            candidate = DriverState::Drowsy;
        } else {
            candidate = DriverState::Distracted;
        }
    } else {
        candidate = DriverState::Safe;
    }

    // Temporal state machine: a change must persist before it takes effect.
    // Escalations react faster (stateEnterSeconds) than recoveries (stateExitSeconds).
    if (candidate != current_) {
        if (candidate != pending_) {
            pending_ = candidate;
            pendingSince_ = tSeconds;
        }
        const double need = severity(candidate) > severity(current_) ? cfg_.stateEnterSeconds
                                                                     : cfg_.stateExitSeconds;
        if (tSeconds - pendingSince_ >= need) current_ = candidate;
    } else {
        pending_ = current_;
    }
    r.state = current_;

    // Alert level from the committed state.
    switch (current_) {
        case DriverState::Safe: r.alertLevel = 0; break;
        case DriverState::AttentionRequired: r.alertLevel = 1; break;
        case DriverState::Drowsy:
        case DriverState::Distracted:
        case DriverState::PhoneUsage:
        case DriverState::HighRisk: r.alertLevel = 2; break;
        case DriverState::Critical: r.alertLevel = 3; break;
    }

    // Reasons (for the panel/timeline), strongest first.
    if (!driverPresent) r.reasons.push_back(distract.message);
    if (!monitoringReliable && driverPresent) r.reasons.push_back("Monitoring quality low");
    if (monitoringReliable && drowsy.level != DrowsyLevel::Alert) {
        r.reasons.push_back(drowsy.message);
    }
    if (distract.level != DistractionLevel::Attentive && driverPresent) {
        r.reasons.push_back(distract.message);
    }
    if (phone.phonePresent) r.reasons.push_back("Phone detected");
    return r;
}

} // namespace dms
