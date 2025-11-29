// src/memory/upload-context.hpp
#pragma once
#include <memory>
#include "utils/types.hpp"

namespace ankh
{

    class CommandPool;
    // Small helper for one-shot uploads / copies.
    // Owns a transient command pool in the given queue family.
    class UploadContext
    {
    public:
        UploadContext(VkDevice device, uint32_t queueFamilyIndex);
        ~UploadContext();

        UploadContext(const UploadContext &) = delete;
        UploadContext &operator=(const UploadContext &) = delete;

        UploadContext(UploadContext &&) = delete;
        UploadContext &operator=(UploadContext &&) = delete;

        // High-level helper: record + submit vkCmdCopyBuffer and wait.
        void copy_buffer(VkQueue queue,
                         VkBuffer src,
                         VkBuffer dst,
                         VkDeviceSize size);

        // Low-level building blocks (useful later if needed).
        VkCommandBuffer begin();
        void endAndSubmit(VkQueue queue, VkCommandBuffer cb);

    private:
        std::unique_ptr<CommandPool> m_pool;
    };

} // namespace ankh
