#pragma once

#include <cstdint>

namespace ankh
{

    struct Config
    {
#ifndef NDEBUG
        bool validation = true;
#else
        bool validation = false;
#endif

        bool vsync = true;      // use FIFO (vsync) vs MAILBOX/IMMEDIATE
        int framesInFlight = 2; // number of frame contexts
        std::uint32_t maxObjects = 4096;
        uint32_t Width = 800;
        uint32_t Height = 600;
    };

    Config &config();

} // namespace ankh
