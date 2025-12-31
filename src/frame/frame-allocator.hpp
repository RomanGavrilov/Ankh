#pragma once

#include "memory/buffer.hpp"
#include "sync/frame-ring.hpp"
#include "utils/gpu-retirement-queue.hpp"
#include <string_view>
#include <vector>

namespace ankh
{

    struct FrameAllocSpan
    {
        VkBuffer buffer{VK_NULL_HANDLE};
        VkDeviceSize offset{0};
        VkDeviceSize size{0};
        std::byte *cpu{nullptr};
    };

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

        ~FrameAllocator() = default;

        FrameAllocator(const FrameAllocator &) = delete;

        FrameAllocator &operator=(const FrameAllocator &) = delete;

        void begin_frame(FrameSlot slot, GpuSignal signal);

        FrameAllocSpan alloc(std::string_view tag, VkDeviceSize size, VkDeviceSize alignment);

        VkBuffer buffer() const noexcept
        {
            return m_buffer->handle();
        }

        VkDeviceSize frame_capacity() const noexcept
        {
            return m_limits.perFrameBytes;
        }

        VkDeviceSize frame_used() const noexcept
        {
            return m_writeHead;
        }

      private:
        VkDeviceSize align_up(VkDeviceSize v, VkDeviceSize a) const noexcept;

        struct DebugAlloc
        {
            std::string tag;
            VkDeviceSize offset;
            VkDeviceSize size;
        };

        Limits m_limits{};
        GpuRetirementQueue *m_retirement{nullptr};

        std::unique_ptr<Buffer> m_buffer;
        std::byte *m_mapped{nullptr};

        VkDeviceSize m_writeHead{0};
        FrameSlot m_currentSlot{};

#ifndef NDEBUG
        std::vector<DebugAlloc> m_debugAllocs;
#endif
    };

} // namespace ankh
