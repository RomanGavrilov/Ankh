#pragma once
#include <cstdint>

namespace ankh
{

    class FrameSync
    {
    public:
        explicit FrameSync(uint32_t frames);
        ~FrameSync();

        uint32_t current() const { return m_current; }
        void advance() { m_current = (m_current + 1) % m_frames; }

    private:
        uint32_t m_frames{2};
        uint32_t m_current{0};
    };

} // namespace ankh
