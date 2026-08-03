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

const char* toString(PhoneState s) {
    switch (s) {
        case PhoneState::NoPhone: return "NO PHONE";
        case PhoneState::Possible: return "POSSIBLE PHONE";
        case PhoneState::Detected: return "PHONE DETECTED";
        case PhoneState::UsageConfirmed: return "PHONE USE CONFIRMED";
    }
    return "?";
}

const char* toString(SmokingState s) {
    switch (s) {
        case SmokingState::NoSmoking: return "NO SMOKING";
        case SmokingState::Possible: return "POSSIBLE SMOKING";
        case SmokingState::Confirmed: return "SMOKING";
        case SmokingState::Unknown: return "UNKNOWN";
    }
    return "?";
}

const char* toString(SeatBeltState s) {
    switch (s) {
        case SeatBeltState::Unknown: return "UNKNOWN";
        case SeatBeltState::NotDetected: return "NOT DETECTED";
        case SeatBeltState::Detected: return "DETECTED";
        case SeatBeltState::Fastened: return "FASTENED";
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
