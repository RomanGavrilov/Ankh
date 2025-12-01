// src/utils/logging.cpp
#include "utils/logging.hpp"

#include <chrono>
#include <iomanip>
#include <iostream>
#include <mutex>

namespace ankh
{
    namespace
    {
        std::mutex g_log_mutex;

        std::string timestamp()
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
            char buf[32];
            std::strftime(buf, sizeof(buf), "%H:%M:%S", &tm);
            return std::string(buf);
        }

        void write_line(const char *level, const std::string &msg)
        {
            std::lock_guard<std::mutex> lock(g_log_mutex);
            std::cerr << "[" << timestamp() << "][" << level << "] " << msg << std::endl;
        }
    } // namespace

    namespace log
    {

        void init() { write_line("INFO", "Logging initialized"); }

        void info(const std::string &msg) { write_line("INFO", msg); }

        void warn(const std::string &msg) { write_line("WARN", msg); }

        void error(const std::string &msg) { write_line("ERROR", msg); }

    } // namespace log

} // namespace ankh
