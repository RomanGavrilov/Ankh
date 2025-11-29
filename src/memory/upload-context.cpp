// src/memory/upload-context.cpp
#include "memory/upload-context.hpp"
#include <stdexcept>

namespace ankh
{

    UploadContext::UploadContext(VkDevice device, uint32_t queueFamilyIndex)
        : m_device(device)
    {
        VkCommandPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
        poolInfo.queueFamilyIndex = queueFamilyIndex;

        if (vkCreateCommandPool(m_device, &poolInfo, nullptr, &m_pool) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create upload command pool");
        }
    }

    UploadContext::~UploadContext()
    {
        if (m_pool != VK_NULL_HANDLE)
        {
            vkDestroyCommandPool(m_device, m_pool, nullptr);
            m_pool = VK_NULL_HANDLE;
        }
    }

    VkCommandBuffer UploadContext::begin()
    {
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = m_pool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = 1;

        VkCommandBuffer cmd{};
        if (vkAllocateCommandBuffers(m_device, &allocInfo, &cmd) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to allocate upload command buffer");
        }

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        if (vkBeginCommandBuffer(cmd, &beginInfo) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to begin upload command buffer");
        }

        return cmd;
    }

    void UploadContext::endAndSubmit(VkQueue queue, VkCommandBuffer cb)
    {
        if (vkEndCommandBuffer(cb) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to end upload command buffer");
        }

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &cb;

        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        VkFence fence{};
        if (vkCreateFence(m_device, &fenceInfo, nullptr, &fence) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create upload fence");
        }

        if (vkQueueSubmit(queue, 1, &submitInfo, fence) != VK_SUCCESS)
        {
            vkDestroyFence(m_device, fence, nullptr);
            throw std::runtime_error("Failed to submit upload command buffer");
        }

        vkWaitForFences(m_device, 1, &fence, VK_TRUE, UINT64_MAX);
        vkDestroyFence(m_device, fence, nullptr);

        vkFreeCommandBuffers(m_device, m_pool, 1, &cb);
        vkResetCommandPool(m_device, m_pool, 0);
    }

    void UploadContext::copy_buffer(VkQueue queue,
                                    VkBuffer src,
                                    VkBuffer dst,
                                    VkDeviceSize size)
    {
        VkCommandBuffer cmd = begin();

        VkBufferCopy copyRegion{};
        copyRegion.srcOffset = 0;
        copyRegion.dstOffset = 0;
        copyRegion.size = size;
        vkCmdCopyBuffer(cmd, src, dst, 1, &copyRegion);

        endAndSubmit(queue, cmd);
    }

} // namespace ankh
