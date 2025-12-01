#include "renderpass/frame-buffer.hpp"
#include <stdexcept>
#include <utils/logging.hpp>

namespace ankh
{

    Framebuffer::Framebuffer(VkDevice device, VkRenderPass render_pass, VkImageView image_view, VkExtent2D extent)
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

        ANKH_VK_CHECK(vkCreateFramebuffer(m_device, &ci, nullptr, &m_framebuffer));
    }

    Framebuffer::Framebuffer(Framebuffer &&other) noexcept
        : m_device(other.m_device)
        , m_framebuffer(other.m_framebuffer)
    {
        other.m_device = {};
        other.m_framebuffer = {};
    }

    Framebuffer &Framebuffer::operator=(Framebuffer &&other) noexcept
    {
        if (this != &other)
        {
            if (m_framebuffer)
                vkDestroyFramebuffer(m_device, m_framebuffer, nullptr);

            m_device = other.m_device;
            m_framebuffer = other.m_framebuffer;

            other.m_device = {};
            other.m_framebuffer = {};
        }
        return *this;
    }

    Framebuffer::~Framebuffer()
    {
        if (m_framebuffer)
            vkDestroyFramebuffer(m_device, m_framebuffer, nullptr);
    }

} // namespace ankh
