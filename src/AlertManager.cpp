#include "dms/AlertManager.hpp"

#include <cstdio>

namespace dms {

namespace {
const char* messageFor(DriverState s) {
    switch (s) {
        case DriverState::Safe: return "";
        case DriverState::AttentionRequired: return "Please pay attention to the road";
        case DriverState::Drowsy: return "Drowsiness detected";
        case DriverState::Distracted: return "Driver distraction detected";
        case DriverState::PhoneUsage: return "Phone usage detected";
        case DriverState::HighRisk: return "High risk - refocus on driving";
        case DriverState::Critical: return "CRITICAL - pull over safely";
    }
    return "";
}
} // namespace

AlertManager::State AlertManager::update(const RiskEngine::Result& risk, double tSeconds) {
    State s;
    s.level = risk.alertLevel;
    s.reasons = risk.reasons;
    s.headline = toString(risk.state);
    s.message = messageFor(risk.state);

    // Audible alarm on level >= 2, rate-limited. Critical (3) pulses ~2x faster.
    if (cfg_.beep && s.level >= 2) {
        const double interval = s.level >= 3 ? cfg_.alertCooldownSeconds * 0.5
                                             : cfg_.alertCooldownSeconds;
        if (tSeconds - lastBeep_ >= interval) {
            std::fputc('\a', stderr);
            std::fflush(stderr);
            lastBeep_ = tSeconds;
        }
    }
    return s;
}

} // namespace dms
