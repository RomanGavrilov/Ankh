#ifndef NDEBUG
#include "frame/frame-allocator.hpp"
#include <cassert>

namespace ankh
{

    // ==== These run automatically in debug builds ====
    static void run_frame_allocator_tests()
    {
        FrameAllocator::Limits limits;
        limits.perFrameBytes = 1024;
        limits.framesInFlight = 2;
        limits.minAlignment = 16;

        FrameAllocator alloc(nullptr, VK_NULL_HANDLE, limits, nullptr);

        alloc.begin_frame(FrameSlot{0}, {});
        auto a = alloc.alloc("A", 100, 1);
        auto b = alloc.alloc("B", 100, 1);

        assert(a.offset % 16 == 0);
        assert(b.offset > a.offset);
    }

    static bool dummy = (run_frame_allocator_tests(), true);

} // namespace ankh
#endif
