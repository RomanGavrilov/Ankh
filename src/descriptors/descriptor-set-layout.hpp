#pragma once
#include "utils/types.hpp"

namespace ankh
{

    class DescriptorSetLayout
    {
    public:
        explicit DescriptorSetLayout(VkDevice device);
        ~DescriptorSetLayout();

        VkDescriptorSetLayout handle() const { return m_layout; }

    private:
        VkDevice m_device{};
        VkDescriptorSetLayout m_layout{};
    };

} // namespace ankh
