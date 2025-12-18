#pragma once
#include "utils/types.hpp"

namespace ankh
{
    class GpuResourceTracker;

    class Framebuffer
    {
      public:
        Framebuffer(VkDevice device,
                    VkRenderPass render_pass,
                    const std::vector<VkImageView> &attachments,
                    VkExtent2D extent,
                    GpuResourceTracker *tracker = nullptr);

        ~Framebuffer();

        Framebuffer(Framebuffer &&other) noexcept;

        Framebuffer &operator=(Framebuffer &&other) noexcept;

        Framebuffer(const Framebuffer &) = delete;
        Framebuffer &operator=(const Framebuffer &) = delete;

        VkFramebuffer handle() const
        {
            return m_framebuffer;
        }

      private:
        VkDevice m_device;
        VkFramebuffer m_framebuffer;
        GpuResourceTracker *m_tracker;
    };

} // namespace ankh
