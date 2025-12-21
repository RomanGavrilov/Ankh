#pragma once
#include <cstdint>

namespace ankh
{
    struct GpuSignal
    {
        enum class Type : uint8_t
        {
            FrameIndex,
            Timeline
        };

        Type type{Type::FrameIndex};
        uint64_t value{0};

        static GpuSignal frame(uint64_t frameIndex) noexcept
        {
            return {Type::FrameIndex, frameIndex};
        }

        static GpuSignal timeline(uint64_t timelineValue) noexcept
        {
            return {Type::Timeline, timelineValue};
        }
    };
} // namespace ankh
