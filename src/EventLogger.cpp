#include "dms/EventLogger.hpp"

#include <sys/stat.h>
#ifdef _WIN32
#include <direct.h>
#endif
#include <ctime>
#include <iostream>

namespace dms {

namespace {
bool pathExists(const std::string& p) {
    struct stat st{};
    return stat(p.c_str(), &st) == 0;
}
} // namespace

EventLogger::EventLogger(const Config& cfg) : cfg_(cfg) {
    // Privacy mode overrides logging entirely: metadata timeline stays in memory
    // but nothing is written to disk.
    if (!cfg_.logEvents || cfg_.privacyMode) return;
#ifdef _WIN32
    _mkdir("logs");
#else
    mkdir("logs", 0755);
#endif
    const std::string path = "logs/events.csv";
    const bool fresh = !pathExists(path);
    csv_.open(path, std::ios::app);
    if (csv_.is_open() && fresh) {
        csv_ << "timestamp,level,event\n";
    }
}

EventLogger::~EventLogger() {
    if (csv_.is_open()) csv_.close();
}

std::string EventLogger::nowStamp() {
    const std::time_t t = std::time(nullptr);
    std::tm tm{};
#ifdef _WIN32
    localtime_s(&tm, &t);
#else
    localtime_r(&t, &tm);
#endif
    char buf[16];
    std::strftime(buf, sizeof(buf), "%H:%M:%S", &tm);
    return buf;
}

void EventLogger::log(const std::string& text, int level) {
    Event e{nowStamp(), text, level};
    events_.push_back(e);
    while (events_.size() > kMaxEvents) events_.pop_front();

    if (csv_.is_open()) {
        csv_ << e.time << ',' << level << ',' << '"' << text << '"' << '\n';
        csv_.flush();
    }
}

} // namespace dms
