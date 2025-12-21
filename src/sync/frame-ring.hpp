#pragma once
#include <cstdint>

namespace ankh
{
    using FrameSlot = uint32_t;

    class FrameRing
    {
      public:
        explicit FrameRing(uint32_t framesInFlight)
            : m_frames(framesInFlight ? framesInFlight : 1u)
            , m_current(0u)
        {
        }

        FrameSlot current() const noexcept
        {
            return m_current;
        }

        uint32_t size() const noexcept
        {
            return m_frames;
        }

        void advance() noexcept
        {
            m_current = (m_current + 1u) % m_frames;
        }

        template <typename F> void for_each_slot(F &&fn) const
        {
            for (uint32_t i = 0; i < m_frames; ++i)
            {
                fn(static_cast<FrameSlot>(i));
            }
        }

      private:
        uint32_t m_frames{1u};
        FrameSlot m_current{0u};
    };
} // namespace ankh
