// src/sync/frame-sync.cpp
#include "sync/frame-sync.hpp"

namespace ankh
{

    FrameSync::FrameSync(uint32_t frames)
        : m_frames(frames)
        , m_current(0)
    {
    }

    FrameSync::~FrameSync() = default;

    uint32_t FrameSync::current() const noexcept
    {
        return m_current;
    }

    uint32_t FrameSync::frames_in_flight() const noexcept
    {
        return m_frames;
    }

    void FrameSync::advance() noexcept
    {
        m_current = (m_current + 1) % m_frames;
    }

} // namespace ankh
