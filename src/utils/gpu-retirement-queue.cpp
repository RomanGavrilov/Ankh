#include "utils/gpu-retirement-queue.hpp"
#include "logging.hpp"

namespace ankh
{
    void GpuRetirementQueue::retire_after(GpuSignal signal, Fn fn)
    {
        if (!fn)
        {
            return;
        }

        if (signal.value == 0)
        {
            fn();
            return;
        }

        std::scoped_lock lock{m_mutex};
        m_items.push_back(Item{signal, std::move(fn)});
    }

    void GpuRetirementQueue::collect(uint64_t completedFrameIndex, uint64_t completedTimelineValue)
    {
        std::vector<Item> ready;

        {
            std::scoped_lock lock{m_mutex};

            size_t write = 0;
            for (size_t i = 0; i < m_items.size(); ++i)
            {
                Item &it = m_items[i];

                const bool done = (it.signal.type == GpuSignal::Type::FrameIndex)
                                      ? (completedFrameIndex >= it.signal.value)
                                      : (completedTimelineValue >= it.signal.value);

                if (done)
                {
                    ready.push_back(std::move(it)); // move-only
                }
                else
                {
                    if (write != i)
                    {
                        m_items[write] = std::move(m_items[i]);
                    }

                    ++write;
                }
            }
            m_items.resize(write);
        }

        for (auto &it : ready)
        {
            if (it.fn)
            {
                it.fn();
            }
            else
            {
                ANKH_THROW_MSG("GpuRetirementQueue::collect null callback");
            }
        }
    }

    void GpuRetirementQueue::flush_all()
    {
        std::vector<Item> all;
        {
            std::scoped_lock lock{m_mutex};
            all = std::move(m_items);
            m_items.clear();
        }

        for (auto &it : all)
        {
            if (it.fn)
            {
                it.fn();
            }
            else
            {
                ANKH_THROW_MSG("GpuRetirementQueue::flush_all null callback");
            }
        }
    }
} // namespace ankh
