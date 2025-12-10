// src/utils/logging.hpp
#pragma once

#include <stdexcept>
#include <string>

namespace ankh
{
    namespace log
    {
        // Initialize logging backend (currently no-op, but hook point for future)
        void init();

        void info(const std::string &msg);
        void warn(const std::string &msg);
        void error(const std::string &msg);
    } // namespace log
} // namespace ankh

#define ANKH_LOG_INFO(msg) ::ankh::log::info(msg)
#define ANKH_LOG_WARN(msg) ::ankh::log::warn(msg)
#define ANKH_LOG_ERROR(msg) ::ankh::log::error(msg)

// Usage: ANKH_VK_CHECK(vkCreateInstance(&info, nullptr, &instance));
#define ANKH_VK_CHECK(expr)                                                                        \
    do                                                                                             \
    {                                                                                              \
        VkResult _ankh_vk_result = (expr);                                                         \
        if (_ankh_vk_result != VK_SUCCESS)                                                         \
        {                                                                                          \
            ::ankh::log::error(std::string("Vulkan call failed: ") + #expr);                       \
            throw std::runtime_error(std::string("Vulkan error from ") + #expr);                   \
        }                                                                                          \
    } while (0)
