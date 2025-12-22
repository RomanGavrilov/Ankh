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
                                             indices.presentFamily.value(),
                                             indices.transferFamily.value()};

        float queuePriority = 1.0f;
        std::vector<VkDeviceQueueCreateInfo> queueInfos;
        queueInfos.reserve(uniqueFamilies.size());

        for (auto family : uniqueFamilies)
        {
            VkDeviceQueueCreateInfo q{};
            q.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            q.queueFamilyIndex = family;
            q.queueCount = 1;
            q.pQueuePriorities = &queuePriority;
            queueInfos.push_back(q);
        }

        // ---------------------------
        // 1) Query feature support
        // ---------------------------
        VkPhysicalDeviceSynchronization2Features sync2Sup{};
        sync2Sup.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES;

        VkPhysicalDeviceTimelineSemaphoreFeatures timelineSup{};
        timelineSup.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TIMELINE_SEMAPHORE_FEATURES;
        timelineSup.pNext = &sync2Sup;

        VkPhysicalDeviceFeatures2 featsSup{};
        featsSup.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
        featsSup.pNext = &timelineSup;

        vkGetPhysicalDeviceFeatures2(phys.handle(), &featsSup);

        ANKH_ASSERT(timelineSup.timelineSemaphore == VK_TRUE);
        ANKH_ASSERT(sync2Sup.synchronization2 == VK_TRUE);

        // ---------------------------
        // 2) Build enable chain
        // ---------------------------
        VkPhysicalDeviceSynchronization2Features sync2{};
        sync2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES;
        sync2.synchronization2 = VK_TRUE;

        VkPhysicalDeviceTimelineSemaphoreFeatures timeline{};
        timeline.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TIMELINE_SEMAPHORE_FEATURES;
        timeline.timelineSemaphore = VK_TRUE;
        timeline.pNext = &sync2;

        VkPhysicalDeviceFeatures2 feats{};
        feats.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
        feats.pNext = &timeline;

        // Core (VkPhysicalDeviceFeatures) go here:
        feats.features.fillModeNonSolid = VK_TRUE;
        feats.features.wideLines = VK_TRUE;
        feats.features.samplerAnisotropy = VK_TRUE;

        // ---------------------------
        // 3) Create device
        // ---------------------------
        const char *extensions[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

        VkDeviceCreateInfo ci{};
        ci.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        ci.queueCreateInfoCount = static_cast<uint32_t>(queueInfos.size());
        ci.pQueueCreateInfos = queueInfos.data();

        // using the features2 path, so don't set pEnabledFeatures
        ci.pEnabledFeatures = nullptr;
        ci.pNext = &feats;

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
        vkGetDeviceQueue(m_device, indices.transferFamily.value(), 0, &m_transfer_queue);
    }

    Device::~Device()
    {
        if (m_device)
            vkDestroyDevice(m_device, nullptr);
    }

} // namespace ankh
