#pragma once

#include <cstdint>

namespace ankh
{

    class Application
    {
    public:
        void run();
        
        // Run the application for a limited number of frames (for testing)
        // Returns true if all frames rendered successfully without crash
        bool run_frames(uint32_t frame_count);
    };

} // namespace ankh
