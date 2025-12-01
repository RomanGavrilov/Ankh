// src/memory/upload-context.cpp
#include "memory/upload-context.hpp"
#include "commands/command-pool.hpp"
#include <stdexcept>

namespace ankh
{

    UploadContext::UploadContext(VkDevice device, uint32_t queueFamilyIndex)
        : m_pool(std::make_unique<CommandPool>(device, queueFamilyIndex))
    {
    }

    UploadContext::~UploadContext() = default;

    VkCommandBuffer UploadContext::begin()
    {
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = m_pool->handle();
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = 1;

        VkCommandBuffer cmd{};
        if (vkAllocateCommandBuffers(m_pool->device(), &allocInfo, &cmd) != VK_SUCCESS)
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
        VkDevice device = m_pool->device();

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
        if (vkCreateFence(device, &fenceInfo, nullptr, &fence) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create upload fence");
        }

        if (vkQueueSubmit(queue, 1, &submitInfo, fence) != VK_SUCCESS)
        {
            vkDestroyFence(device, fence, nullptr);
            throw std::runtime_error("Failed to submit upload command buffer");
        }

        vkWaitForFences(device, 1, &fence, VK_TRUE, UINT64_MAX);
        vkDestroyFence(device, fence, nullptr);

        vkFreeCommandBuffers(device, m_pool->handle(), 1, &cb);
        vkResetCommandPool(device, m_pool->handle(), 0);
    }

    void UploadContext::copy_buffer(VkQueue queue, VkBuffer src, VkBuffer dst, VkDeviceSize size)
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
