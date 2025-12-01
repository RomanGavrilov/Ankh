#include "sync/sync-primitives.hpp"
#include <stdexcept>
#include <utils/logging.hpp>

namespace ankh
{

    SyncPrimitives::SyncPrimitives(VkDevice device, uint32_t frames)
        : m_device(device)
    {

        m_image_available.resize(frames);
        m_render_finished.resize(frames);
        m_in_flight_fences.resize(frames);

        VkSemaphoreCreateInfo si{};
        si.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fi{};
        fi.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fi.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        for (uint32_t i = 0; i < frames; ++i)
        {
            ANKH_VK_CHECK(vkCreateSemaphore(m_device, &si, nullptr, &m_image_available[i]));
            ANKH_VK_CHECK(vkCreateSemaphore(m_device, &si, nullptr, &m_render_finished[i]));
            ANKH_VK_CHECK(vkCreateFence(m_device, &fi, nullptr, &m_in_flight_fences[i]));
        }
    }

    SyncPrimitives::~SyncPrimitives()
    {
        for (auto fence : m_in_flight_fences)
        {
            vkDestroyFence(m_device, fence, nullptr);
        }

        for (auto s : m_render_finished)
        {
            vkDestroySemaphore(m_device, s, nullptr);
        }

        for (auto s : m_image_available)
        {
            vkDestroySemaphore(m_device, s, nullptr);
        }
    }

} // namespace ankh
