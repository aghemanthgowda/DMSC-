#pragma once

#include "dms/Config.hpp"

#include <deque>
#include <fstream>
#include <string>

namespace dms {

// Keeps a rolling in-memory list of notable events for the dashboard timeline
// and (optionally) appends them to logs/events.csv. Video is never stored.
class EventLogger {
public:
    struct Event {
        std::string time;   // wall-clock HH:MM:SS
        std::string text;
        int level = 0;      // 0 info, 1 warn, 2 alert, 3 critical
    };

    explicit EventLogger(const Config& cfg);
    ~EventLogger();

    // Record an event (also written to CSV when logging is enabled).
    void log(const std::string& text, int level = 0);

    // Most-recent events (newest last), capped for the timeline panel.
    const std::deque<Event>& recent() const { return events_; }

private:
    Config cfg_;
    std::deque<Event> events_;
    std::ofstream csv_;
    static constexpr size_t kMaxEvents = 40;

    static std::string nowStamp();
};

} // namespace dms
