#pragma once
#include "utils/types.hpp"

namespace ankh
{

    class PhysicalDevice
    {
    public:
        PhysicalDevice(VkInstance instance, VkSurfaceKHR surface);

        VkPhysicalDevice handle() const { return m_device; }
        QueueFamilyIndices queues() const { return m_indices; }

    private:
        VkPhysicalDevice m_device{};
        QueueFamilyIndices m_indices;
    };

} // namespace ankh
