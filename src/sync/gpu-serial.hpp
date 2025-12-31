// src/sync/gpu-serial.hpp
#pragma once

#include "sync/frame-ring.hpp"
#include <cstdint>
#include <vector>

namespace ankh
{
    using GpuSerialValue = uint64_t;

    class GpuSerial
    {
      public:
        explicit GpuSerial(uint32_t framesInFlight)
            : m_lastUsed(framesInFlight ? framesInFlight : 1u, 0)
        {
        }

        GpuSerialValue issue() noexcept
        {
            return ++m_next;
        }

        void mark_slot_used(FrameSlot slot, GpuSerialValue serial) noexcept
        {
            m_lastUsed[slot % m_lastUsed.size()] = serial;
        }

        void mark_slot_completed(FrameSlot slot) noexcept
        {
            const GpuSerialValue done = m_lastUsed[slot % m_lastUsed.size()];
            if (done > m_completed)
            {
                m_completed = done;
            }
        }

        GpuSerialValue completed() const noexcept
        {
            return m_completed;
        }

        GpuSerialValue last_issued() const noexcept
        {
            return m_next;
        }

      private:
        GpuSerialValue m_next{0};
        GpuSerialValue m_completed{0};
        std::vector<GpuSerialValue> m_lastUsed; // per-slot last used serial
    };
} // namespace ankh
