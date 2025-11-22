#pragma once
#include "utils/types.hpp"
#include <vector>

namespace ankh
{

    class SyncPrimitives
    {
    public:
        SyncPrimitives(VkDevice device, uint32_t frames);
        ~SyncPrimitives();

        const std::vector<VkSemaphore> &image_available() const { return m_image_available; }
        const std::vector<VkSemaphore> &render_finished() const { return m_render_finished; }
        const std::vector<VkFence> &in_flight_fences() const { return m_in_flight_fences; }

    private:
        VkDevice m_device{};

        std::vector<VkSemaphore> m_image_available;
        std::vector<VkSemaphore> m_render_finished;
        std::vector<VkFence> m_in_flight_fences;
    };

} // namespace ankh
