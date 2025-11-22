#pragma once
#include "utils/Types.hpp"

namespace ankh
{

    class SyncPrimitives
    {
    public:
        SyncPrimitives();
        ~SyncPrimitives();
    };

    class FrameSync
    {
    public:
        FrameSync();
        ~FrameSync();

        uint32_t currentFrame() const;
    };

} // namespace ankh