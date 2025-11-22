#pragma once
#include "utils/types.hpp"

namespace ankh
{

    class Framebuffer
    {
    public:
        Framebuffer(VkDevice device,
                    VkRenderPass render_pass,
                    VkImageView image_view,
                    VkExtent2D extent);
        ~Framebuffer();

        VkFramebuffer handle() const { return m_framebuffer; }

    private:
        VkDevice m_device{};
        VkFramebuffer m_framebuffer{};
    };

} // namespace ankh
