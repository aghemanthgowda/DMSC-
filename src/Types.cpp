#include "dms/Types.hpp"

namespace dms {

const char* toString(DrowsyLevel l) {
    switch (l) {
        case DrowsyLevel::Alert: return "ALERT";
        case DrowsyLevel::Possible: return "POSSIBLE DROWSINESS";
        case DrowsyLevel::Drowsy: return "DROWSY";
        case DrowsyLevel::Critical: return "CRITICAL DROWSINESS";
    }
    return "?";
}

const char* toString(DistractionLevel l) {
    switch (l) {
        case DistractionLevel::Attentive: return "ATTENTIVE";
        case DistractionLevel::Brief: return "BRIEFLY DISTRACTED";
        case DistractionLevel::Distracted: return "DISTRACTED";
        case DistractionLevel::Highly: return "HIGHLY DISTRACTED";
    }
    return "?";
}

const char* toString(DriverState s) {
    switch (s) {
        case DriverState::Safe: return "SAFE";
        case DriverState::AttentionRequired: return "ATTENTION REQUIRED";
        case DriverState::Drowsy: return "DROWSY";
        case DriverState::Distracted: return "DISTRACTED";
        case DriverState::PhoneUsage: return "PHONE USAGE";
        case DriverState::HighRisk: return "HIGH RISK";
        case DriverState::Critical: return "CRITICAL";
    }
    return "?";
}

} // namespace dms
