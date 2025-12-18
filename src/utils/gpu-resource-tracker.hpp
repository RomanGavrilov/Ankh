#pragma once

#include <cstdint>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

namespace ankh
{

#ifndef NDEBUG

    class GpuResourceTracker
    {
      public:
        struct Record
        {
            std::string type;
            std::string name;
            const char *file{nullptr};
            int line{0};
        };

        void
        on_create(const char *type, uint64_t handle, const char *name, const char *file, int line);

        void on_destroy(uint64_t handle);

        void report_leaks() const;

        bool empty() const;

      private:
        mutable std::mutex m_mutex;
        std::unordered_map<uint64_t, Record> m_live;
    };

#else

    class GpuResourceTracker
    {
      public:
        void on_create(const char *, uint64_t, const char *, const char *, int) {}
        void on_destroy(uint64_t) {}
        void report_leaks() const {}
        bool empty() const
        {
            return true;
        }
    };
#endif

} // namespace ankh
