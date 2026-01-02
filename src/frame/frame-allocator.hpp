#pragma once

#include "memory/buffer.hpp"
#include "utils/types.hpp"
#include "utils/gpu-signal.hpp"
#include "sync/frame-ring.hpp"

#include <cstddef>
#include <memory>
#include <string>
#include <string_view>
#include <vector>
 

namespace ankh
{
    class GpuRetirementQueue;

    struct FrameAllocSpan
    {
        VkBuffer buffer{VK_NULL_HANDLE};
        VkDeviceSize offset{0};
        VkDeviceSize size{0};
        void *cpu{nullptr};
    };

#ifndef NDEBUG
    struct FrameAllocDebug
    {
        std::string tag;
        VkDeviceSize offset{0};
        VkDeviceSize size{0};
    };
#endif

    class FrameAllocator
    {
      public:
        struct Limits
        {
            VkDeviceSize perFrameBytes{0};
            uint32_t framesInFlight{0};
            VkDeviceSize minAlignment{1};
        };

        FrameAllocator(VmaAllocator allocator,
                       VkDevice device,
                       Limits limits,
                       GpuRetirementQueue *retirement);

        void begin_frame(FrameSlot slot, GpuSignal signal);

        FrameAllocSpan alloc(std::string_view tag, VkDeviceSize size, VkDeviceSize alignment);

        VkBuffer buffer() const
        {
            return m_buffer ? m_buffer->handle() : VK_NULL_HANDLE;
        }

        VkDeviceSize frame_capacity() const
        {
            return m_limits.perFrameBytes;
        }

        VkDeviceSize total_capacity() const
        {
            return m_limits.perFrameBytes * static_cast<VkDeviceSize>(m_limits.framesInFlight);
        }

        VkDeviceSize frame_base_offset() const
        {
            return m_frameBase;
        }

#ifndef NDEBUG
        VkDeviceSize frame_used() const
        {
            return m_writeHead;
        }
        const std::vector<FrameAllocDebug> &debug_allocs() const
        {
            return m_debugAllocs;
        }
#endif

      private:
        VkDeviceSize align_up(VkDeviceSize v, VkDeviceSize a) const noexcept;

      private:
        Limits m_limits{};
        GpuRetirementQueue *m_retirement{nullptr};

        std::unique_ptr<Buffer> m_buffer{};
        std::byte *m_mapped{nullptr};

        FrameSlot m_currentSlot{0};
        VkDeviceSize m_writeHead{0};

        
        VkDeviceSize m_frameBase{0};

#ifndef NDEBUG
        std::vector<FrameAllocDebug> m_debugAllocs;
#endif
    };

} // namespace ankh
