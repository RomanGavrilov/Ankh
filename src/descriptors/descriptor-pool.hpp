#pragma once
#include "utils/Types.hpp"

namespace ankh
{

    class DescriptorPool
    {
    public:
        DescriptorPool();
        ~DescriptorPool();

        VkDescriptorPool handle() const;
    };

} // namespace ankh