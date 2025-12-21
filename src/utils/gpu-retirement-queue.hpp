#pragma once

#include "utils/gpu-signal.hpp"
#include <cstdint>
#include <functional> // std::move_only_function
#include <mutex>
#include <vector>

namespace ankh
{
    class GpuRetirementQueue
    {
      public:
        using Fn = std::move_only_function<void()>;

        void retire_after(GpuSignal signal, Fn fn);

        void collect(uint64_t completedFrameIndex, uint64_t completedTimelineValue);

        void flush_all();

      private:
        struct Item
        {
            GpuSignal signal{};
            Fn fn;
        };

        std::mutex m_mutex;
        std::vector<Item> m_items;
    };
} // namespace ankh
