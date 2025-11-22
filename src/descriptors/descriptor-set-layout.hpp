#pragma once
#include "utils/Types.hpp"

namespace ankh
{

    class DescriptorSetLayout
    {
    public:
        DescriptorSetLayout();
        ~DescriptorSetLayout();

        VkDescriptorSetLayout handle() const;
    };

} // namespace ankh