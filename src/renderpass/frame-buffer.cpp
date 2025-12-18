#include "renderpass/frame-buffer.hpp"
#include "utils/gpu-tracking.hpp"
#include "utils/logging.hpp"
#include <stdexcept>

namespace ankh
{

    Framebuffer::Framebuffer(VkDevice device,
                             VkRenderPass render_pass,
                             const std::vector<VkImageView> &attachments,
                             VkExtent2D extent,
                             GpuResourceTracker *tracker)
        : m_device{device}
        , m_tracker{tracker}
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
        ANKH_GPU_TRACK_CREATE(m_tracker, "VkFramebuffer", m_framebuffer, "Framebuffer");
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

            m_tracker = other.m_tracker;
            other.m_tracker = nullptr;
        }

        return *this;
    }

    Framebuffer::~Framebuffer()
    {
        ANKH_GPU_TRACK_DESTROY(m_tracker, m_framebuffer);

        if (m_framebuffer)
        {
            vkDestroyFramebuffer(m_device, m_framebuffer, nullptr);
        }
    }

} // namespace ankh
