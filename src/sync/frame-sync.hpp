// src/sync/frame-sync.hpp
#pragma once
#include <cstdint>

namespace ankh
{

    class FrameSync
    {
      public:
        explicit FrameSync(uint32_t frames);
        ~FrameSync();

        uint32_t current() const noexcept;
        uint32_t frames_in_flight() const noexcept;
        void advance() noexcept;

        // Monotonic frame/epoch tracking

        uint64_t next_frame_id() noexcept;
        void mark_frame_used(uint32_t slot, uint64_t frame_id) noexcept;
        void mark_frame_complete(uint32_t slot) noexcept;

      private:
        uint32_t m_frames{2};
        uint32_t m_current{0};
    };

} // namespace ankh
