#include "dms/ConfigManager.hpp"

#include <opencv2/core.hpp>

#include <iostream>

namespace dms {

namespace {
// Read a key into `v` only if it exists in the file (otherwise keep default).
template <typename T>
void readInto(const cv::FileStorage& fs, const char* key, T& v) {
    const cv::FileNode n = fs[key];
    if (!n.empty()) {
        T tmp{};
        n >> tmp;
        v = tmp;
    }
}
void readBool(const cv::FileStorage& fs, const char* key, bool& v) {
    const cv::FileNode n = fs[key];
    if (!n.empty()) v = static_cast<int>(n) != 0;
}
} // namespace

bool ConfigManager::load(const std::string& path, Config& cfg) {
    cv::FileStorage fs;
    try {
        fs.open(path, cv::FileStorage::READ);
    } catch (const cv::Exception&) {
        return false;
    }
    if (!fs.isOpened()) return false;

    readInto(fs, "ear_threshold", cfg.earThreshold);
    readInto(fs, "ear_close_ratio", cfg.earCloseRatio);
    readInto(fs, "eye_closed_alarm_seconds", cfg.eyeClosedAlarmSeconds);
    readInto(fs, "eye_closed_drowsy_seconds", cfg.eyeClosedDrowsySeconds);
    readInto(fs, "perclos_window_seconds", cfg.perclosWindowSeconds);
    readInto(fs, "perclos_warn", cfg.perclosWarn);
    readInto(fs, "perclos_alarm", cfg.perclosAlarm);
    readInto(fs, "long_blink_seconds", cfg.longBlinkSeconds);
    readInto(fs, "mar_threshold", cfg.marThreshold);
    readInto(fs, "yawn_min_seconds", cfg.yawnMinSeconds);
    readInto(fs, "head_away_yaw_degrees", cfg.headAwayYawDegrees);
    readInto(fs, "head_down_pitch_degrees", cfg.headDownPitchDegrees);
    readInto(fs, "head_away_duration_seconds", cfg.headAwayDurationSeconds);
    readInto(fs, "gaze_off_threshold", cfg.gazeOffThreshold);
    readInto(fs, "phone_confidence_threshold", cfg.phoneConfidenceThreshold);
    readInto(fs, "phone_confirm_frames", cfg.phoneConfirmFrames);
    readInto(fs, "phone_detect_every_n_frames", cfg.phoneDetectEveryNFrames);
    readInto(fs, "face_lost_grace_seconds", cfg.faceLostGraceSeconds);
    readInto(fs, "no_face_alarm_seconds", cfg.noFaceAlarmSeconds);
    readInto(fs, "w_drowsiness", cfg.wDrowsiness);
    readInto(fs, "w_distraction", cfg.wDistraction);
    readInto(fs, "w_phone", cfg.wPhone);
    readInto(fs, "w_yawn", cfg.wYawn);
    readInto(fs, "risk_warning_threshold", cfg.riskWarnThreshold);
    readInto(fs, "risk_high_threshold", cfg.riskHighThreshold);
    readInto(fs, "risk_critical_threshold", cfg.riskCriticalThreshold);
    readInto(fs, "state_enter_seconds", cfg.stateEnterSeconds);
    readInto(fs, "state_exit_seconds", cfg.stateExitSeconds);
    readInto(fs, "alert_cooldown_seconds", cfg.alertCooldownSeconds);
    readInto(fs, "calibration_seconds", cfg.calibrationSeconds);
    readInto(fs, "camera_index", cfg.cameraIndex);
    readInto(fs, "capture_width", cfg.captureWidth);
    readInto(fs, "capture_height", cfg.captureHeight);
    readInto(fs, "phone_model", cfg.phoneModel);
    readBool(fs, "mirror", cfg.mirror);
    readBool(fs, "beep", cfg.beep);
    readBool(fs, "developer_mode", cfg.developerMode);
    readBool(fs, "log_events", cfg.logEvents);

    fs.release();
    std::cout << "[ConfigManager] Loaded config from " << path << "\n";
    return true;
}

bool ConfigManager::writeDefault(const std::string& path, const Config& cfg) {
    cv::FileStorage fs;
    try {
        fs.open(path, cv::FileStorage::WRITE);
    } catch (const cv::Exception&) {
        return false;
    }
    if (!fs.isOpened()) return false;

    fs << "ear_threshold" << cfg.earThreshold;
    fs << "ear_close_ratio" << cfg.earCloseRatio;
    fs << "eye_closed_alarm_seconds" << cfg.eyeClosedAlarmSeconds;
    fs << "eye_closed_drowsy_seconds" << cfg.eyeClosedDrowsySeconds;
    fs << "perclos_window_seconds" << cfg.perclosWindowSeconds;
    fs << "perclos_warn" << cfg.perclosWarn;
    fs << "perclos_alarm" << cfg.perclosAlarm;
    fs << "long_blink_seconds" << cfg.longBlinkSeconds;
    fs << "mar_threshold" << cfg.marThreshold;
    fs << "yawn_min_seconds" << cfg.yawnMinSeconds;
    fs << "head_away_yaw_degrees" << cfg.headAwayYawDegrees;
    fs << "head_down_pitch_degrees" << cfg.headDownPitchDegrees;
    fs << "head_away_duration_seconds" << cfg.headAwayDurationSeconds;
    fs << "gaze_off_threshold" << cfg.gazeOffThreshold;
    fs << "phone_confidence_threshold" << cfg.phoneConfidenceThreshold;
    fs << "phone_confirm_frames" << cfg.phoneConfirmFrames;
    fs << "phone_detect_every_n_frames" << cfg.phoneDetectEveryNFrames;
    fs << "face_lost_grace_seconds" << cfg.faceLostGraceSeconds;
    fs << "no_face_alarm_seconds" << cfg.noFaceAlarmSeconds;
    fs << "w_drowsiness" << cfg.wDrowsiness;
    fs << "w_distraction" << cfg.wDistraction;
    fs << "w_phone" << cfg.wPhone;
    fs << "w_yawn" << cfg.wYawn;
    fs << "risk_warning_threshold" << cfg.riskWarnThreshold;
    fs << "risk_high_threshold" << cfg.riskHighThreshold;
    fs << "risk_critical_threshold" << cfg.riskCriticalThreshold;
    fs << "state_enter_seconds" << cfg.stateEnterSeconds;
    fs << "state_exit_seconds" << cfg.stateExitSeconds;
    fs << "alert_cooldown_seconds" << cfg.alertCooldownSeconds;
    fs << "calibration_seconds" << cfg.calibrationSeconds;
    fs << "camera_index" << cfg.cameraIndex;
    fs << "capture_width" << cfg.captureWidth;
    fs << "capture_height" << cfg.captureHeight;
    fs << "phone_model" << cfg.phoneModel;
    fs << "mirror" << (cfg.mirror ? 1 : 0);
    fs << "beep" << (cfg.beep ? 1 : 0);
    fs << "developer_mode" << (cfg.developerMode ? 1 : 0);
    fs << "log_events" << (cfg.logEvents ? 1 : 0);

    fs.release();
    return true;
}

} // namespace dms
