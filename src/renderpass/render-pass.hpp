#pragma once
#include "utils/types.hpp"

namespace ankh
{

    class RenderPass
    {
    public:
        RenderPass(VkDevice device, VkFormat swapchain_format);
        ~RenderPass();

        VkRenderPass handle() const { return m_render_pass; }

    private:
        VkDevice m_device{};
        VkRenderPass m_render_pass{};
    };

} // namespace ankh
