// src/memory/upload-context.cpp
#include "upload-context.hpp"
#include <stdexcept>

namespace ankh
{
    UploadContext::UploadContext(VkDevice device, uint32_t qFamily) : m_device(device)
    {
        VkCommandPoolCreateInfo pi{};
        pi.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        pi.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        pi.queueFamilyIndex = qFamily;
        if (vkCreateCommandPool(device, &pi, nullptr, &m_pool) != VK_SUCCESS)
            throw std::runtime_error("upload command pool create failed");
    }
    UploadContext::~UploadContext()
    {
        if (m_pool)
            vkDestroyCommandPool(m_device, m_pool, nullptr);
    }

    VkCommandBuffer UploadContext::begin()
    {
        VkCommandBufferAllocateInfo ai{};
        ai.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        ai.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        ai.commandPool = m_pool;
        ai.commandBufferCount = 1;
        VkCommandBuffer cb{};
        vkAllocateCommandBuffers(m_device, &ai, &cb);
        VkCommandBufferBeginInfo bi{};
        bi.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        bi.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        vkBeginCommandBuffer(cb, &bi);
        return cb;
    }

    void UploadContext::endAndSubmit(VkQueue queue, VkCommandBuffer cb)
    {
        vkEndCommandBuffer(cb);
        VkSubmitInfo si{};
        si.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        si.commandBufferCount = 1;
        si.pCommandBuffers = &cb;
        vkQueueSubmit(queue, 1, &si, VK_NULL_HANDLE);
        vkQueueWaitIdle(queue);
        vkFreeCommandBuffers(m_device, m_pool, 1, &cb);
    }

} // namespace ankh