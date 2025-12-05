#pragma once

namespace ankh
{

    struct Config
    {
        bool validation = true;
        bool vsync = true;      // use FIFO (vsync) vs MAILBOX/IMMEDIATE
        int framesInFlight = 2; // number of frame contexts
    };

    Config &config();

} // namespace ankh
