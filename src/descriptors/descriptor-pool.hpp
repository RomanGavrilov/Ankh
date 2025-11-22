#pragma once
#include "utils/types.hpp"

namespace ankh
{

    class DescriptorPool
    {
    public:
        DescriptorPool(VkDevice device, uint32_t max_sets);
        ~DescriptorPool();

        VkDescriptorPool handle() const { return m_pool; }

    private:
        VkDevice m_device{};
        VkDescriptorPool m_pool{};
    };

} // namespace ankh
