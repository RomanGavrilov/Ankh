// src/utils/logging.cpp
#include "utils/logging.hpp"

#include <chrono>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <mutex>

namespace
{
    std::mutex g_log_mutex;

    std::string current_time_string()
    {
        using clock = std::chrono::system_clock;
        auto now = clock::now();
        auto t = clock::to_time_t(now);

        std::tm tm{};
#if defined(_WIN32)
        localtime_s(&tm, &t);
#else
        localtime_r(&t, &tm);
#endif

        std::ostringstream oss;
        oss << std::put_time(&tm, "%H:%M:%S");
        return oss.str();
    }

    void write_log(const char *level, const std::string &msg, bool error_stream)
    {
        std::lock_guard<std::mutex> lock(g_log_mutex);

        auto &os = error_stream ? std::cerr : std::cout;
        os << "[" << current_time_string() << "][" << level << "] " << msg << '\n';
    }
} // namespace

namespace ankh::log
{
    // For now this is just a hook for future routing (file, platform logger, etc.).
    void init() {}

    void info(const std::string &msg)
    {
        write_log("INFO", msg, false);
    }

    void warn(const std::string &msg)
    {
        write_log("WARN", msg, false);
    }

    void error(const std::string &msg)
    {
        write_log("ERROR", msg, true);
    }

} // namespace ankh::log
