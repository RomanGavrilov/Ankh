#pragma once
#include "utils/types.hpp"
#include <cstdint>
#include <utils/config.hpp>
#include <vector>

namespace ankh
{
    struct UploadTicket
    {
        uint64_t value = 0;
    };

    class AsyncUploader
    {
      public:
        static constexpr uint32_t UPLOAD_CONTEXTS{3};

        AsyncUploader(VkDevice device, uint32_t queueFamilyIndex, VkQueue queue);

        ~AsyncUploader();

        AsyncUploader(const AsyncUploader &) = delete;
        AsyncUploader &operator=(const AsyncUploader &) = delete;
        AsyncUploader(AsyncUploader &&) = delete;
        AsyncUploader &operator=(AsyncUploader &&) = delete;

        // Begin an upload batch
        void begin();

        // Record operations (record into internal command buffer)
        // These take raw Vulkan handles and regions to keep the uploader backend-agnostic.
        void copy_buffer(VkBuffer src, VkBuffer dst, const VkBufferCopy &region);

        // Convenience overload
        void copy_buffer(VkBuffer src,
                         VkBuffer dst,
                         VkDeviceSize size,
                         VkDeviceSize srcOffset = 0,
                         VkDeviceSize dstOffset = 0);

        // End and submit batch. Signals timeline semaphore to returned ticket.value.
        UploadTicket end_and_submit();

        // Completed timeline semaphore value
        uint64_t completed_value() const;

        VkSemaphore timeline_semaphore() const
        {
            return m_timeline;
        }

        void transition_image_layout(VkImage image,
                                     VkImageAspectFlags aspectMask,
                                     VkImageLayout oldLayout,
                                     VkImageLayout newLayout,
                                     uint32_t baseMipLevel = 0,
                                     uint32_t levelCount = 1,
                                     uint32_t baseArrayLayer = 0,
                                     uint32_t layerCount = 1);

        void copy_buffer_to_image(VkBuffer src,
                                  VkImage dst,
                                  VkImageLayout dstLayout,
                                  const VkBufferImageCopy &region);

        void copy_buffer_to_image(VkBuffer src,
                                  VkImage dst,
                                  VkImageLayout dstLayout,
                                  uint32_t width,
                                  uint32_t height,
                                  VkImageAspectFlags aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                                  uint32_t mipLevel = 0,
                                  uint32_t baseArrayLayer = 0,
                                  uint32_t layerCount = 1);

      private:
        VkDevice m_device = VK_NULL_HANDLE;
        
        VkQueue m_queue = VK_NULL_HANDLE;

        // Ring of command pools/buffers
        std::vector<VkCommandPool> m_pools;
        
        std::vector<VkCommandBuffer> m_commands;
        
        std::vector<uint64_t> m_in_flight_value; // last signaled timeline value for that slot
        
        uint32_t m_current = 0;

        // current cmd (points to m_commands[m_current] while recording)
        VkCommandBuffer m_command = VK_NULL_HANDLE;

        VkSemaphore m_timeline = VK_NULL_HANDLE;
        
        uint64_t m_nextSignal = 0;

        bool m_recording = false;
    };
} // namespace ankh
