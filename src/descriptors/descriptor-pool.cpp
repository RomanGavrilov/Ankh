#include "descriptors/descriptor-pool.hpp"
#include <stdexcept>

namespace ankh
{

    DescriptorPool::DescriptorPool(VkDevice device, uint32_t max_sets)
        : m_device(device)
    {

        VkDescriptorPoolSize poolSize{};
        poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        poolSize.descriptorCount = max_sets;

        VkDescriptorPoolCreateInfo ci{};
        ci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        ci.poolSizeCount = 1;
        ci.pPoolSizes = &poolSize;
        ci.maxSets = max_sets;

        if (vkCreateDescriptorPool(m_device, &ci, nullptr, &m_pool) != VK_SUCCESS)
            throw std::runtime_error("failed to create descriptor pool");
    }

    DescriptorPool::~DescriptorPool()
    {
        if (m_pool)
            vkDestroyDescriptorPool(m_device, m_pool, nullptr);
    }

} // namespace ankh
