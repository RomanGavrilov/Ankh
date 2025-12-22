#include "core/physical-device.hpp"
#include <set>
#include <stdexcept>
#include <utils/logging.hpp>
#include <vector>

namespace ankh
{

    static bool check_device_extension_support(VkPhysicalDevice device)
    {
        uint32_t extensionCount = 0;
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

        std::vector<VkExtensionProperties> available(extensionCount);
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, available.data());

        std::set<std::string> required = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

        for (const auto &ext : available)
            required.erase(ext.extensionName);

        return required.empty();
    }

    static SwapChainSupportDetails query_swapchain_support(VkPhysicalDevice device,
                                                           VkSurfaceKHR surface)
    {
        SwapChainSupportDetails details{};
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

        uint32_t formatCount = 0;
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
        if (formatCount)
        {
            details.formats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(device,
                                                 surface,
                                                 &formatCount,
                                                 details.formats.data());
        }

        uint32_t presentModeCount = 0;
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);
        if (presentModeCount)
        {
            details.presentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(device,
                                                      surface,
                                                      &presentModeCount,
                                                      details.presentModes.data());
        }

        return details;
    }

    static QueueFamilyIndices find_queue_families(VkPhysicalDevice device, VkSurfaceKHR surface)
    {
        QueueFamilyIndices indices{};

        uint32_t count = 0;

        vkGetPhysicalDeviceQueueFamilyProperties(device, &count, nullptr);

        std::vector<VkQueueFamilyProperties> families(count);

        vkGetPhysicalDeviceQueueFamilyProperties(device, &count, families.data());

        for (uint32_t i = 0; i < count; ++i)
        {
            if (families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
            {
                indices.graphicsFamily = i;
            }

            VkBool32 presentSupport = VK_FALSE;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

            if (presentSupport)
            {
                indices.presentFamily = i;
            }

            // Some devices have a dedicated transfer queue, others do not
            // Prefer a dedicated one if available
            if (families[i].queueFlags & VK_QUEUE_TRANSFER_BIT)
            {
                indices.transferFamily = i;
            }

            if (indices.isComplete())
            {
                break;
            }
        }

        if (!indices.transferFamily.has_value())
        {
            // Fallback to graphics queue if no dedicated transfer queue found
            indices.transferFamily = indices.graphicsFamily;
        }

        return indices;
    }

    PhysicalDevice::PhysicalDevice(VkInstance instance, VkSurfaceKHR surface)
    {
        uint32_t deviceCount = 0;

        vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

        if (!deviceCount)
        {
            ANKH_THROW_MSG("failed to find GPUs with Vulkan support");
        }

        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

        for (auto dev : devices)
        {
            auto indices = find_queue_families(dev, surface);

            if (!indices.isComplete())
            {
                continue;
            }

            if (!check_device_extension_support(dev))
            {
                continue;
            }

            auto swapSupport = query_swapchain_support(dev, surface);
            if (swapSupport.formats.empty() || swapSupport.presentModes.empty())
            {
                continue;
            }

            m_device = dev;
            m_indices = indices;
            break;
        }

        if (!m_device)
        {
            ANKH_THROW_MSG("failed to find a suitable GPU");
        }
    }

} // namespace ankh
