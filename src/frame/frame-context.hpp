#pragma once
#include "utils/Types.hpp"

namespace ankh
{

    class FrameContext
    {
    public:
        FrameContext();
        ~FrameContext();

        uint32_t imageIndex() const;
    };

} // namespace ankh