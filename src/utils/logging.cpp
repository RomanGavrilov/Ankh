// src/utils/logging.cpp
#include "utils/logging.hpp"

#include <chrono>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <mutex>

namespace ankh::log
{
    namespace
    {
        Level g_level = Level::Debug;
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

        void write_log(Level level, const std::string &msg, bool error_stream)
        {

            if (level < g_level)
            {
                return;
            }

            std::lock_guard<std::mutex> lock(g_log_mutex);

            const auto prefix = [&]()
            {
                switch (level)
                {
                case Level::Debug:
                    return "DEBUG";
                case Level::Info:
                    return "INFO";
                case Level::Warn:
                    return "WARN";
                case Level::Error:
                    return "ERROR";
                default:
                    return "UNKNOWN";
                }
            }();

            auto &os = error_stream ? std::cerr : std::cout;
            os << "[" << current_time_string() << "][" << prefix << "] " << msg << '\n';
        }

    } // namespace

    void SetLevel(Level level)
    {
        g_level = level;
    }

    void debug(const std::string &msg)
    {
        write_log(Level::Debug, msg, false);
    }

    // For now this is just a hook for future routing (file, platform logger, etc.).
    void init() {}

    void info(const std::string &msg)
    {
        write_log(Level::Info, msg, false);
    }

    void warn(const std::string &msg)
    {
        write_log(Level::Warn, msg, false);
    }

    void error(const std::string &msg)
    {
        write_log(Level::Error, msg, true);
    }

} // namespace ankh::log
