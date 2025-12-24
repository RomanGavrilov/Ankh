#include "streaming/async-uploader.hpp"
#include <utils/logging.hpp>

namespace ankh
{

    namespace
    {
        struct Sync2Access
        {
            VkPipelineStageFlags2 stage = VK_PIPELINE_STAGE_2_NONE;
            VkAccessFlags2 access = VK_ACCESS_2_NONE;
        };

        // Given an image layout, what pipeline stages and memory accesses should be synced against?
        Sync2Access access_for_layout(VkImageLayout layout)
        {
            switch (layout)
            {
            case VK_IMAGE_LAYOUT_UNDEFINED:
                return {VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT, VK_ACCESS_2_NONE};

            case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
                return {VK_PIPELINE_STAGE_2_TRANSFER_BIT, VK_ACCESS_2_TRANSFER_WRITE_BIT};

            case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
                return {VK_PIPELINE_STAGE_2_TRANSFER_BIT, VK_ACCESS_2_TRANSFER_READ_BIT};

            case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
                return {VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT |
                            VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT,
                        VK_ACCESS_2_SHADER_SAMPLED_READ_BIT | VK_ACCESS_2_SHADER_READ_BIT};

            case VK_IMAGE_LAYOUT_GENERAL:
                return {VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
                        VK_ACCESS_2_SHADER_READ_BIT | VK_ACCESS_2_SHADER_WRITE_BIT};

            case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
                return {VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
                        VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT |
                            VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT};

            case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
                return {VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT |
                            VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT,
                        VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                            VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT};

            default:
                // Safe fallback
                return {VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
                        VK_ACCESS_2_MEMORY_READ_BIT | VK_ACCESS_2_MEMORY_WRITE_BIT};
            }
        }
    } // namespace

    AsyncUploader::AsyncUploader(VkDevice device, uint32_t queueFamilyIndex, VkQueue queue)
        : m_device(device)
        , m_queue(queue)
    {
        // Command pool
        VkCommandPoolCreateInfo pci{};
        pci.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        pci.flags =
            VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

        pci.queueFamilyIndex = queueFamilyIndex;
        m_pools.resize(UPLOAD_CONTEXTS, VK_NULL_HANDLE);
        m_commands.resize(UPLOAD_CONTEXTS, VK_NULL_HANDLE);
        m_in_flight_value.resize(UPLOAD_CONTEXTS, 0ull);
        m_current = UPLOAD_CONTEXTS - 1;

        for (uint32_t i = 0; i < UPLOAD_CONTEXTS; ++i)
        {
            ANKH_VK_CHECK(vkCreateCommandPool(m_device, &pci, nullptr, &m_pools[i]));
        }

        for (uint32_t i = 0; i < UPLOAD_CONTEXTS; ++i)
        {
            VkCommandBufferAllocateInfo ai{};
            ai.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            ai.commandPool = m_pools[i];
            ai.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            ai.commandBufferCount = 1;
            ANKH_VK_CHECK(vkAllocateCommandBuffers(m_device, &ai, &m_commands[i]));
        }

        m_command = m_commands[m_current];

        // Timeline semaphore
        VkSemaphoreTypeCreateInfo typeInfo{};
        typeInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO;
        typeInfo.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;
        typeInfo.initialValue = 0;

        VkSemaphoreCreateInfo sci{};
        sci.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        sci.pNext = &typeInfo;
        ANKH_VK_CHECK(vkCreateSemaphore(m_device, &sci, nullptr, &m_timeline));
    }

    AsyncUploader::~AsyncUploader()
    {
        // Wait for all pending upload operations to complete before destroying resources
        if (m_timeline && m_nextSignal > 0)
        {
            VkSemaphoreWaitInfo wi{VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO};
            wi.semaphoreCount = 1;
            wi.pSemaphores = &m_timeline;
            wi.pValues = &m_nextSignal;
            vkWaitSemaphores(m_device, &wi, UINT64_MAX);
        }

        if (m_timeline)
        {
            vkDestroySemaphore(m_device, m_timeline, nullptr);
        }

        for (uint32_t i = 0; i < UPLOAD_CONTEXTS; ++i)
        {
            if (m_pools[i] != VK_NULL_HANDLE)
            {
                if (m_commands[i] != VK_NULL_HANDLE)
                {
                    vkFreeCommandBuffers(m_device, m_pools[i], 1, &m_commands[i]);
                }

                vkDestroyCommandPool(m_device, m_pools[i], nullptr);
            }
        }
    }

    void AsyncUploader::begin()
    {
        ANKH_ASSERT(!m_recording);

        m_current = (m_current + 1) % UPLOAD_CONTEXTS;

        const uint64_t need = m_in_flight_value[m_current];
        if (need != 0)
        {
            VkSemaphoreWaitInfo wi{VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO};
            wi.semaphoreCount = 1;
            wi.pSemaphores = &m_timeline;
            wi.pValues = &need;
            ANKH_VK_CHECK(vkWaitSemaphores(m_device, &wi, UINT64_MAX));
        }

        // Now safe to reset and record
        ANKH_VK_CHECK(vkResetCommandPool(m_device, m_pools[m_current], 0));

        VkCommandBufferBeginInfo bi{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
        bi.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        ANKH_VK_CHECK(vkBeginCommandBuffer(m_commands[m_current], &bi));

        m_command = m_commands[m_current];
        m_recording = true;
    }

    void AsyncUploader::copy_buffer(VkBuffer src, VkBuffer dst, const VkBufferCopy &region)
    {
        ANKH_ASSERT(m_recording);
        vkCmdCopyBuffer(m_command, src, dst, 1, &region);
    }

    void AsyncUploader::copy_buffer(VkBuffer src,
                                    VkBuffer dst,
                                    VkDeviceSize size,
                                    VkDeviceSize srcOffset,
                                    VkDeviceSize dstOffset)
    {
        VkBufferCopy region{};
        region.srcOffset = srcOffset;
        region.dstOffset = dstOffset;
        region.size = size;
        copy_buffer(src, dst, region);
    }

    UploadTicket AsyncUploader::end_and_submit()
    {
        ANKH_ASSERT(m_recording);
        ANKH_VK_CHECK(vkEndCommandBuffer(m_command));
        m_recording = false;

        const uint64_t signalValue = ++m_nextSignal;

        VkCommandBufferSubmitInfo cmdInfo{};
        cmdInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
        cmdInfo.commandBuffer = m_command;

        VkSemaphoreSubmitInfo signalInfo{};
        signalInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
        signalInfo.semaphore = m_timeline;
        signalInfo.value = signalValue;

        // For "signal when everything in this submit is done":
        signalInfo.stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
        signalInfo.deviceIndex = 0;

        VkSubmitInfo2 submit{};
        submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
        submit.commandBufferInfoCount = 1;
        submit.pCommandBufferInfos = &cmdInfo;
        submit.signalSemaphoreInfoCount = 1;
        submit.pSignalSemaphoreInfos = &signalInfo;

        ANKH_VK_CHECK(vkQueueSubmit2(m_queue, 1, &submit, VK_NULL_HANDLE));

        // This ring slot is now busy until timeline reaches signalValue
        m_in_flight_value[m_current] = signalValue;

        return UploadTicket{signalValue};
    }

    uint64_t AsyncUploader::completed_value() const
    {
        uint64_t v = 0;
        ANKH_VK_CHECK(vkGetSemaphoreCounterValue(m_device, m_timeline, &v));
        return v;
    }

    void ankh::AsyncUploader::transition_image_layout(VkImage image,
                                                      VkImageAspectFlags aspectMask,
                                                      VkImageLayout oldLayout,
                                                      VkImageLayout newLayout,
                                                      uint32_t baseMipLevel,
                                                      uint32_t levelCount,
                                                      uint32_t baseArrayLayer,
                                                      uint32_t layerCount)
    {
        ANKH_ASSERT(m_recording);

        const Sync2Access src = access_for_layout(oldLayout);
        const Sync2Access dst = access_for_layout(newLayout);

        VkImageMemoryBarrier2 barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
        barrier.srcStageMask = src.stage;
        barrier.srcAccessMask = src.access;
        barrier.dstStageMask = dst.stage;
        barrier.dstAccessMask = dst.access;
        barrier.oldLayout = oldLayout;
        barrier.newLayout = newLayout;
        barrier.image = image;
        barrier.subresourceRange.aspectMask = aspectMask;
        barrier.subresourceRange.baseMipLevel = baseMipLevel;
        barrier.subresourceRange.levelCount = levelCount;
        barrier.subresourceRange.baseArrayLayer = baseArrayLayer;
        barrier.subresourceRange.layerCount = layerCount;

        // If you use a dedicated transfer queue later, you may need queue-family ownership
        // transfers here.
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

        VkDependencyInfo dep{};
        dep.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
        dep.imageMemoryBarrierCount = 1;
        dep.pImageMemoryBarriers = &barrier;

        vkCmdPipelineBarrier2(m_command, &dep);
    }

    void AsyncUploader::copy_buffer_to_image(VkBuffer src,
                                             VkImage dst,
                                             VkImageLayout dstLayout,
                                             const VkBufferImageCopy &region)
    {
        ANKH_ASSERT(m_recording);
        vkCmdCopyBufferToImage(m_command, src, dst, dstLayout, 1, &region);
    }

    void AsyncUploader::copy_buffer_to_image(VkBuffer src,
                                             VkImage dst,
                                             VkImageLayout dstLayout,
                                             uint32_t width,
                                             uint32_t height,
                                             VkImageAspectFlags aspectMask,
                                             uint32_t mipLevel,
                                             uint32_t baseArrayLayer,
                                             uint32_t layerCount)
    {
        ANKH_ASSERT(m_recording);

        VkBufferImageCopy region{};
        region.bufferOffset = 0;
        region.bufferRowLength = 0;   // tightly packed
        region.bufferImageHeight = 0; // tightly packed

        region.imageSubresource.aspectMask = aspectMask;
        region.imageSubresource.mipLevel = mipLevel;
        region.imageSubresource.baseArrayLayer = baseArrayLayer;
        region.imageSubresource.layerCount = layerCount;

        region.imageOffset = {0, 0, 0};
        region.imageExtent = {width, height, 1};

        copy_buffer_to_image(src, dst, dstLayout, region);
    }

} // namespace ankh
