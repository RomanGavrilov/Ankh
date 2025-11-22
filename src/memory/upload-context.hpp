#pragma once
#include "utils/types.hpp"

namespace ankh
{

    class UploadContext
    {
    public:
        UploadContext(VkDevice device, uint32_t queueFamilyIndex);
        ~UploadContext();

        VkCommandBuffer begin();
        void endAndSubmit(VkQueue queue, VkCommandBuffer cb);

    private:
        VkDevice m_device{};
        VkCommandPool m_pool{};
    };

} // namespace ankh
