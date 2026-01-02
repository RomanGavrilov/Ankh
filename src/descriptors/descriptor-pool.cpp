// src/descriptors/descriptor-pool.cpp
#include "descriptor-pool.hpp"

#include <stdexcept>
#include <utils/logging.hpp>

namespace ankh
{

    DescriptorPool::DescriptorPool(VkDevice device, uint32_t max_sets)
        : m_device(device)
    {
        VkDescriptorPoolSize poolSizes[3]{};

        // dynamic uniform buffers
        poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
        poolSizes[0].descriptorCount = max_sets;

        // dynamic storage buffers
        poolSizes[1].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
        poolSizes[1].descriptorCount = max_sets;

        // combined image samplers
        poolSizes[2].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        poolSizes[2].descriptorCount = max_sets;

        VkDescriptorPoolCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        info.poolSizeCount = 3;
        info.pPoolSizes = poolSizes;
        info.maxSets = max_sets;

        ANKH_VK_CHECK(vkCreateDescriptorPool(m_device, &info, nullptr, &m_pool));
    }

    DescriptorPool::~DescriptorPool()
    {
        if (m_pool != VK_NULL_HANDLE)
        {
            vkDestroyDescriptorPool(m_device, m_pool, nullptr);
            m_pool = VK_NULL_HANDLE;
        }
    }

} // namespace ankh
