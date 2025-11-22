#pragma once
#include "utils/Types.hpp"

namespace ankh
{

    class PhysicalDevice
    {
    public:
        PhysicalDevice();
        ~PhysicalDevice();

        VkPhysicalDevice handle() const;
        QueueFamilyIndices queues() const;
    };

} // namespace ankh