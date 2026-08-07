#pragma once

#include <opencv2/videoio.hpp>

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <thread>

namespace dms {

// Decouples camera capture from processing so latency stays bounded on a slow
// device. A background thread reads frames as fast as the camera delivers them
// and keeps ONLY the most recent one; the processing loop always pulls that
// latest frame and lets older ones be dropped.
//
// Without this, if per-frame processing is slower than the camera's frame rate,
// the driver's internal buffer backs up and the displayed video falls further
// and further behind real time (the "huge lag" symptom). Dropping stale frames
// keeps what you see live — the metrics are wall-clock based, so skipped frames
// don't distort PERCLOS/blink/yawn timing.
class CameraGrabber {
public:
    ~CameraGrabber() { stop(); }

    // Takes over the (already-opened) capture. The grabber thread is the only
    // thing that touches `cap` from here on.
    void start(cv::VideoCapture& cap) {
        cap_ = &cap;
        // Ask the backend to keep as few buffered frames as possible.
        cap_->set(cv::CAP_PROP_BUFFERSIZE, 1);
        running_.store(true);
        thread_ = std::thread([this] { loop(); });
    }

    // Block until a frame is available (or capture has ended), then copy the
    // latest frame into `out`. Returns false when the camera has stopped.
    bool read(cv::Mat& out) {
        std::unique_lock<std::mutex> lk(mtx_);
        cv_.wait(lk, [this] { return hasFrame_ || ended_.load(); });
        if (!hasFrame_) return false;      // ended with nothing to give
        out = latest_;                     // hand over; next grab allocates anew
        hasFrame_ = false;
        return !ended_.load() || !out.empty();
    }

    void stop() {
        if (!running_.exchange(false)) return;
        ended_.store(true);
        cv_.notify_all();
        if (thread_.joinable()) thread_.join();
    }

private:
    void loop() {
        cv::Mat frame;
        while (running_.load()) {
            if (!cap_->read(frame) || frame.empty()) {
                ended_.store(true);
                cv_.notify_all();
                break;
            }
            {
                std::lock_guard<std::mutex> lk(mtx_);
                latest_ = frame.clone();   // replace whatever was pending
                hasFrame_ = true;
            }
            cv_.notify_one();
        }
    }

    cv::VideoCapture* cap_ = nullptr;
    std::thread thread_;
    std::mutex mtx_;
    std::condition_variable cv_;
    cv::Mat latest_;
    bool hasFrame_ = false;
    std::atomic<bool> running_{false};
    std::atomic<bool> ended_{false};
};

} // namespace dms
