#pragma once

namespace ankh
{

    struct Config
    {
        bool validation = true;
        bool vsync = true;
        int framesInFlight = 2;
    };

} // namespace ankh