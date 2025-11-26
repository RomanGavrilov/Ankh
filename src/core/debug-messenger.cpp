#include "core/debug-messenger.hpp"
#include <iostream>
#include <atomic>

namespace ankh
{
    // Static counters for validation errors and warnings
    static std::atomic<uint32_t> s_error_count{0};
    static std::atomic<uint32_t> s_warning_count{0};

    uint32_t DebugMessenger::error_count()
    {
        return s_error_count.load();
    }

    uint32_t DebugMessenger::warning_count()
    {
        return s_warning_count.load();
    }

    void DebugMessenger::reset_counters()
    {
        s_error_count.store(0);
        s_warning_count.store(0);
    }

    static VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT,
        const VkDebugUtilsMessengerCallbackDataEXT *callbackData,
        void *)
    {

        if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
        {
            s_error_count.fetch_add(1);
            std::cerr << "validation error: " << callbackData->pMessage << std::endl;
        }
        else if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
        {
            s_warning_count.fetch_add(1);
            std::cerr << "validation warning: " << callbackData->pMessage << std::endl;
        }
        return VK_FALSE;
    }

    DebugMessenger::DebugMessenger(VkInstance instance)
        : m_instance(instance)
    {

        if (!kEnableValidation)
            return;

        auto vkCreateDebugUtilsMessengerEXT =
            reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
                vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"));

        if (!vkCreateDebugUtilsMessengerEXT)
            return;

        VkDebugUtilsMessengerCreateInfoEXT ci{};
        ci.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        ci.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                             VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                             VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        ci.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                         VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                         VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        ci.pfnUserCallback = debug_callback;

        if (vkCreateDebugUtilsMessengerEXT(m_instance, &ci, nullptr, &m_messenger) != VK_SUCCESS)
            throw std::runtime_error("failed to create debug messenger");
    }

    DebugMessenger::~DebugMessenger()
    {
        if (!kEnableValidation || !m_messenger)
            return;

        auto vkDestroyDebugUtilsMessengerEXT =
            reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
                vkGetInstanceProcAddr(m_instance, "vkDestroyDebugUtilsMessengerEXT"));

        if (vkDestroyDebugUtilsMessengerEXT)
            vkDestroyDebugUtilsMessengerEXT(m_instance, m_messenger, nullptr);
    }

} // namespace ankh
