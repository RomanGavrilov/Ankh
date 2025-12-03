#pragma once
#include "utils/types.hpp"

namespace ankh
{

    class Framebuffer
    {
      public:
        Framebuffer(VkDevice device, VkRenderPass render_pass, const std::vector<VkImageView> &attachments, VkExtent2D extent);

        ~Framebuffer();

        Framebuffer(Framebuffer &&other) noexcept;

        Framebuffer &operator=(Framebuffer &&other) noexcept;

        Framebuffer(const Framebuffer &) = delete;
        Framebuffer &operator=(const Framebuffer &) = delete;

        VkFramebuffer handle() const { return m_framebuffer; }

      private:
        VkDevice m_device{VK_NULL_HANDLE};
        VkFramebuffer m_framebuffer{VK_NULL_HANDLE};
    };

} // namespace ankh
