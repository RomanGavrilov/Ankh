#include "renderpass/frame-buffer.hpp"
#include <stdexcept>

namespace ankh
{

    Framebuffer::Framebuffer(VkDevice device,
                             VkRenderPass render_pass,
                             VkImageView image_view,
                             VkExtent2D extent)
        : m_device(device)
    {

        VkImageView attachments[] = {image_view};

        VkFramebufferCreateInfo ci{};
        ci.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        ci.renderPass = render_pass;
        ci.attachmentCount = 1;
        ci.pAttachments = attachments;
        ci.width = extent.width;
        ci.height = extent.height;
        ci.layers = 1;

        if (vkCreateFramebuffer(m_device, &ci, nullptr, &m_framebuffer) != VK_SUCCESS)
            throw std::runtime_error("failed to create framebuffer");
    }

    Framebuffer::~Framebuffer()
    {
        if (m_framebuffer)
            vkDestroyFramebuffer(m_device, m_framebuffer, nullptr);
    }

} // namespace ankh
