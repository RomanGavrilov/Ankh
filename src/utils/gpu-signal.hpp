#pragma once
#include <cstdint>

namespace ankh
{
    // GPU signal for synchronization and resource retirement
    // Can represent either a frame index or a timeline semaphore value
    // Used to track when GPU operations associated with a resource are complete
    // and it is safe to reclaim or reuse that resource
    struct GpuSignal
    {
        enum class Type : uint8_t
        {
            FrameIndex,
            Timeline
        };

        Type type{Type::FrameIndex};
        uint64_t value{0};

        static GpuSignal frame(uint64_t frameId) noexcept
        {
            return {Type::FrameIndex, frameId};
        }

        static GpuSignal timeline(uint64_t timelineValue) noexcept
        {
            return {Type::Timeline, timelineValue};
        }
    };
} // namespace ankh
