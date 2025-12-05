#pragma once

#include <cstdint>

namespace ankh
{

    struct Config
    {
        bool validation = true;
        bool vsync = true;      // use FIFO (vsync) vs MAILBOX/IMMEDIATE
        int framesInFlight = 2; // number of frame contexts
        std::uint32_t maxObjects = 4096;
    };

    Config &config();

} // namespace ankh
