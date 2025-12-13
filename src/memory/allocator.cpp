#include "memory/allocator.hpp"
#include "utils/logging.hpp"
#include <stdexcept>

namespace ankh
{
    Allocator::Allocator(VkInstance instance, VkPhysicalDevice phys, VkDevice device)
    {
        VmaAllocatorCreateInfo ci{};
        ci.instance = instance;
        ci.physicalDevice = phys;
        ci.device = device;

        ANKH_VK_CHECK(vmaCreateAllocator(&ci, &m_allocator));
        ANKH_LOG_DEBUG("[Allocator] VMA allocator created");
    }

    Allocator::~Allocator()
    {
        if (m_allocator)
        {
            char *stats = nullptr;
            vmaBuildStatsString(m_allocator, &stats, VK_TRUE);
            if (stats)
            {
                ANKH_LOG_ERROR(std::string("[Allocator] Stats:\n") + stats);
                vmaFreeStatsString(m_allocator, stats);
            }

            vmaDestroyAllocator(m_allocator);
            m_allocator = VK_NULL_HANDLE;
        }
    }
} // namespace ankh
