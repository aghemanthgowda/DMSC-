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
    readInto(fs, "mar_open_delta", cfg.marOpenDelta);
    readInto(fs, "mar_baseline_window_seconds", cfg.marBaselineWindowSeconds);
    readInto(fs, "yawn_min_seconds", cfg.yawnMinSeconds);
    readInto(fs, "yawn_dip_tolerance_seconds", cfg.yawnDipToleranceSeconds);
    readInto(fs, "face_detect_every_n_frames", cfg.faceDetectEveryNFrames);
    readInto(fs, "detection_scale", cfg.detectionScale);
    readInto(fs, "head_away_yaw_degrees", cfg.headAwayYawDegrees);
    readInto(fs, "head_down_pitch_degrees", cfg.headDownPitchDegrees);
    readInto(fs, "head_away_duration_seconds", cfg.headAwayDurationSeconds);
    readInto(fs, "gaze_off_threshold", cfg.gazeOffThreshold);
    readInto(fs, "class_id_phone", cfg.classIdPhone);
    readInto(fs, "class_id_person", cfg.classIdPerson);
    readInto(fs, "class_id_cigarette", cfg.classIdCigarette);
    readInto(fs, "class_id_seatbelt", cfg.classIdSeatbelt);
    readInto(fs, "yolo_confidence", cfg.yoloConfidence);
    readInto(fs, "nms_threshold", cfg.nmsThreshold);
    readInto(fs, "object_track_timeout_seconds", cfg.objectTrackTimeoutSeconds);
    readInto(fs, "hand_phone_distance_fraction", cfg.handPhoneDistanceFraction);
    readInto(fs, "hand_mouth_distance_fraction", cfg.handMouthDistanceFraction);
    readInto(fs, "phone_confidence_threshold", cfg.phoneConfidenceThreshold);
    readInto(fs, "phone_confirm_frames", cfg.phoneConfirmFrames);
    readInto(fs, "phone_detect_every_n_frames", cfg.phoneDetectEveryNFrames);
    readBool(fs, "enable_hand_detection", cfg.enableHandDetection);
    readInto(fs, "hand_min_area_fraction", cfg.handMinAreaFraction);
    readInto(fs, "hand_confirm_seconds", cfg.handConfirmSeconds);
    readInto(fs, "face_lost_grace_seconds", cfg.faceLostGraceSeconds);
    readInto(fs, "no_face_alarm_seconds", cfg.noFaceAlarmSeconds);
    readInto(fs, "quality_min_face_width_fraction", cfg.qualityMinFaceWidthFraction);
    readInto(fs, "quality_low_light_mean", cfg.qualityLowLightMean);
    readInto(fs, "quality_min_confidence", cfg.qualityMinConfidence);
    readInto(fs, "quality_max_yaw_degrees", cfg.qualityMaxYawDegrees);
    readInto(fs, "quality_max_pitch_degrees", cfg.qualityMaxPitchDegrees);
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
    readInto(fs, "camera_rotation", cfg.cameraRotation);
    readBool(fs, "head_pose_calibrate", cfg.headPoseCalibrate);
    readInto(fs, "head_yaw_offset", cfg.headYawOffset);
    readInto(fs, "head_pitch_offset", cfg.headPitchOffset);
    readInto(fs, "head_roll_offset", cfg.headRollOffset);
    readInto(fs, "phone_model", cfg.phoneModel);
    readBool(fs, "mirror", cfg.mirror);
    readBool(fs, "beep", cfg.beep);
    readBool(fs, "developer_mode", cfg.developerMode);
    readBool(fs, "log_events", cfg.logEvents);
    readBool(fs, "privacy_mode", cfg.privacyMode);
    readBool(fs, "headless", cfg.headless);

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
    fs << "mar_open_delta" << cfg.marOpenDelta;
    fs << "mar_baseline_window_seconds" << cfg.marBaselineWindowSeconds;
    fs << "yawn_min_seconds" << cfg.yawnMinSeconds;
    fs << "yawn_dip_tolerance_seconds" << cfg.yawnDipToleranceSeconds;
    fs << "face_detect_every_n_frames" << cfg.faceDetectEveryNFrames;
    fs << "detection_scale" << cfg.detectionScale;
    fs << "head_away_yaw_degrees" << cfg.headAwayYawDegrees;
    fs << "head_down_pitch_degrees" << cfg.headDownPitchDegrees;
    fs << "head_away_duration_seconds" << cfg.headAwayDurationSeconds;
    fs << "gaze_off_threshold" << cfg.gazeOffThreshold;
    fs << "class_id_phone" << cfg.classIdPhone;
    fs << "class_id_person" << cfg.classIdPerson;
    fs << "class_id_cigarette" << cfg.classIdCigarette;
    fs << "class_id_seatbelt" << cfg.classIdSeatbelt;
    fs << "yolo_confidence" << cfg.yoloConfidence;
    fs << "nms_threshold" << cfg.nmsThreshold;
    fs << "object_track_timeout_seconds" << cfg.objectTrackTimeoutSeconds;
    fs << "hand_phone_distance_fraction" << cfg.handPhoneDistanceFraction;
    fs << "hand_mouth_distance_fraction" << cfg.handMouthDistanceFraction;
    fs << "phone_confidence_threshold" << cfg.phoneConfidenceThreshold;
    fs << "phone_confirm_frames" << cfg.phoneConfirmFrames;
    fs << "phone_detect_every_n_frames" << cfg.phoneDetectEveryNFrames;
    fs << "enable_hand_detection" << (cfg.enableHandDetection ? 1 : 0);
    fs << "hand_min_area_fraction" << cfg.handMinAreaFraction;
    fs << "hand_confirm_seconds" << cfg.handConfirmSeconds;
    fs << "face_lost_grace_seconds" << cfg.faceLostGraceSeconds;
    fs << "no_face_alarm_seconds" << cfg.noFaceAlarmSeconds;
    fs << "quality_min_face_width_fraction" << cfg.qualityMinFaceWidthFraction;
    fs << "quality_low_light_mean" << cfg.qualityLowLightMean;
    fs << "quality_min_confidence" << cfg.qualityMinConfidence;
    fs << "quality_max_yaw_degrees" << cfg.qualityMaxYawDegrees;
    fs << "quality_max_pitch_degrees" << cfg.qualityMaxPitchDegrees;
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
    fs << "camera_rotation" << cfg.cameraRotation;
    fs << "head_pose_calibrate" << (cfg.headPoseCalibrate ? 1 : 0);
    fs << "head_yaw_offset" << cfg.headYawOffset;
    fs << "head_pitch_offset" << cfg.headPitchOffset;
    fs << "head_roll_offset" << cfg.headRollOffset;
    fs << "phone_model" << cfg.phoneModel;
    fs << "mirror" << (cfg.mirror ? 1 : 0);
    fs << "beep" << (cfg.beep ? 1 : 0);
    fs << "developer_mode" << (cfg.developerMode ? 1 : 0);
    fs << "log_events" << (cfg.logEvents ? 1 : 0);
    fs << "privacy_mode" << (cfg.privacyMode ? 1 : 0);
    fs << "headless" << (cfg.headless ? 1 : 0);

    fs.release();
    return true;
}

} // namespace dms
