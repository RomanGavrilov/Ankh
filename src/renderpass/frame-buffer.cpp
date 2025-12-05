#include "renderpass/frame-buffer.hpp"
#include <stdexcept>
#include <utils/logging.hpp>

namespace ankh
{

    Framebuffer::Framebuffer(VkDevice device,
                             VkRenderPass render_pass,
                             const std::vector<VkImageView> &attachments,
                             VkExtent2D extent)
        : m_device(device)
    {

        VkFramebufferCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        info.renderPass = render_pass;
        info.attachmentCount = static_cast<uint32_t>(attachments.size());
        info.pAttachments = attachments.data();
        info.width = extent.width;
        info.height = extent.height;
        info.layers = 1;

        ANKH_VK_CHECK(vkCreateFramebuffer(m_device, &info, nullptr, &m_framebuffer));
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
            {
                vkDestroyFramebuffer(m_device, m_framebuffer, nullptr);
            }

            m_device = other.m_device;
            m_framebuffer = other.m_framebuffer;

            other.m_device = VK_NULL_HANDLE;
            other.m_framebuffer = VK_NULL_HANDLE;
        }
        return *this;
    }

    Framebuffer::~Framebuffer()
    {
        if (m_framebuffer)
        {
            vkDestroyFramebuffer(m_device, m_framebuffer, nullptr);
        }
    }

} // namespace ankh
