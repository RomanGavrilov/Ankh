#include "core/device.hpp"
#include "core/physical-device.hpp"
#include "utils/config.hpp"
#include "utils/logging.hpp"
#include <set>
#include <stdexcept>

namespace ankh
{

    Device::Device(const PhysicalDevice &phys)
    {
        QueueFamilyIndices indices = phys.queues();

        std::set<uint32_t> uniqueFamilies = {indices.graphicsFamily.value(),
                                             indices.presentFamily.value()};

        float queuePriority = 1.0f;
        std::vector<VkDeviceQueueCreateInfo> queueInfos;

        for (auto family : uniqueFamilies)
        {
            VkDeviceQueueCreateInfo q{};
            q.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            q.queueFamilyIndex = family;
            q.queueCount = 1;
            q.pQueuePriorities = &queuePriority;
            queueInfos.push_back(q);
        }

        VkPhysicalDeviceFeatures features{.fillModeNonSolid = VK_TRUE,
                                          .wideLines = VK_TRUE,
                                          .samplerAnisotropy = VK_TRUE};

        const char *extensions[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

        VkDeviceCreateInfo ci{};
        ci.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        ci.queueCreateInfoCount = static_cast<uint32_t>(queueInfos.size());
        ci.pQueueCreateInfos = queueInfos.data();
        ci.pEnabledFeatures = &features;
        ci.enabledExtensionCount = 1;
        ci.ppEnabledExtensionNames = extensions;

        const char *layers[] = {"VK_LAYER_KHRONOS_validation"};
        if (ankh::config().validation)
        {
            ci.enabledLayerCount = 1;
            ci.ppEnabledLayerNames = layers;
        }

        ANKH_VK_CHECK(vkCreateDevice(phys.handle(), &ci, nullptr, &m_device));
        ANKH_LOG_DEBUG("[Device] Created logical device");

        vkGetDeviceQueue(m_device, indices.graphicsFamily.value(), 0, &m_graphics_queue);
        vkGetDeviceQueue(m_device, indices.presentFamily.value(), 0, &m_present_queue);
    }

    Device::~Device()
    {
        if (m_device)
            vkDestroyDevice(m_device, nullptr);
    }

} // namespace ankh
