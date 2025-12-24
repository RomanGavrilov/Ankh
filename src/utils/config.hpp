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
        const uint32_t uploadContexts = 2; // number of async upload contexts
        
        // 16ms in nanoseconds:
        const uint64_t acquireImageTimeoutNs = 16ull * 1000ull * 1000ull;
    };

    Config &config();

} // namespace ankh
