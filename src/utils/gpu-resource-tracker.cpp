#include "utils/gpu-resource-tracker.hpp"
#include "utils/logging.hpp"

namespace ankh
{

#ifndef NDEBUG

    void GpuResourceTracker::on_create(const char *type,
                                       uint64_t handle,
                                       const char *name,
                                       const char *file,
                                       int line)
    {
        if (0 == handle)
        {
            return;
        }

        std::scoped_lock lock(m_mutex);

        m_live.emplace(handle,
                       Record{.type = type, .name = name ? name : "", .file = file, .line = line});
    }

    void GpuResourceTracker::on_destroy(uint64_t handle)
    {
        if (0 == handle)
        {
            return;
        }

        std::scoped_lock lock(m_mutex);
        m_live.erase(handle);
    }

    bool GpuResourceTracker::empty() const
    {
        std::scoped_lock lock(m_mutex);
        return m_live.empty();
    }

    void GpuResourceTracker::report_leaks() const
    {
        std::scoped_lock lock(m_mutex);

        if (m_live.empty())
        {
            ANKH_LOG_DEBUG("GPU Resource Tracker: no leaks detected");
            return;
        }

        ANKH_LOG_ERROR("GPU Resource Tracker: leaked GPU resources detected:");

        for (const auto &[handle, rec] : m_live)
        {
            ANKH_LOG_ERROR("  Leak: " + rec.type + " handle=0x" + std::to_string(handle) +
                           " name=\"" + rec.name + "\"" + " (" + (rec.file ? rec.file : "?") + ":" +
                           std::to_string(rec.line) + ")");
        }
    }

#endif

} // namespace ankh
