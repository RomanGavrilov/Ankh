#include "frame/frame-allocator.hpp"
#include "utils/logging.hpp"

namespace ankh
{

    FrameAllocator::FrameAllocator(VmaAllocator allocator,
                                   VkDevice device,
                                   Limits limits,
                                   GpuRetirementQueue *retirement)
        : m_limits(limits)
        , m_retirement(retirement)
    {
        ANKH_ASSERT(limits.perFrameBytes > 0);
        ANKH_ASSERT(limits.framesInFlight > 0);

        const VkDeviceSize totalSize = limits.perFrameBytes * limits.framesInFlight;

        m_buffer = std::make_unique<Buffer>(allocator,
                                            device,
                                            totalSize,
                                            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT |
                                                VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                                            VMA_MEMORY_USAGE_CPU_TO_GPU,
                                            retirement,
                                            GpuSignal{},
                                            VMA_ALLOCATION_CREATE_MAPPED_BIT);

        m_mapped = reinterpret_cast<std::byte *>(m_buffer->map());
    }

    void FrameAllocator::begin_frame(FrameSlot slot, GpuSignal signal)
    {
        m_currentSlot = slot;
        m_writeHead = 0;

#ifndef NDEBUG
        m_debugAllocs.clear();
#endif

        if (m_retirement)
        {
            m_buffer->set_retirement(m_retirement, signal);
        }
    }

    VkDeviceSize FrameAllocator::align_up(VkDeviceSize v, VkDeviceSize a) const noexcept
    {
        return (v + a - 1) & ~(a - 1);
    }

    FrameAllocSpan
    FrameAllocator::alloc(std::string_view tag, VkDeviceSize size, VkDeviceSize alignment)
    {
        alignment = std::max(alignment, m_limits.minAlignment);

        VkDeviceSize aligned = align_up(m_writeHead, alignment);
        VkDeviceSize end = aligned + size;

        if (end > m_limits.perFrameBytes)
        {
#ifndef NDEBUG
            ANKH_LOG_ERROR("[FrameAllocator] OVERFLOW\n"
                           "  tag: {}\n"
                           "  requested: {}\n"
                           "  remaining: {}\n"
                           "  capacity: {}",
                           tag,
                           size,
                           m_limits.perFrameBytes - m_writeHead,
                           m_limits.perFrameBytes);
            ANKH_ASSERT(false && "FrameAllocator overflow");
#endif
            return {};
        }

        const VkDeviceSize frameBase =
            static_cast<VkDeviceSize>(m_currentSlot) * m_limits.perFrameBytes;

        FrameAllocSpan span;
        span.buffer = m_buffer->handle();
        span.offset = frameBase + aligned;
        span.size = size;
        span.cpu = m_mapped + span.offset;

        m_writeHead = end;

#ifndef NDEBUG
        m_debugAllocs.push_back({std::string(tag), span.offset, size});
#endif

        return span;
    }

} // namespace ankh
