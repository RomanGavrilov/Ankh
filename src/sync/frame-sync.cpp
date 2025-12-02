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

} // namespace ankh
