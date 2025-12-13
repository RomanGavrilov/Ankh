// src/core/context.cpp
#include "core/context.hpp"

#include "core/debug-messenger.hpp"
#include "core/device.hpp"
#include "core/instance.hpp"
#include "core/physical-device.hpp"
#include "platform/surface.hpp"
#include "utils/config.hpp"

#include <stdexcept>

namespace ankh
{

    Context::Context(GLFWwindow *window)
    {
        m_instance = std::make_unique<Instance>();

        if (ankh::config().validation)
        {
            m_debug_messenger = std::make_unique<DebugMessenger>(m_instance->handle());
        }

        m_surface = std::make_unique<Surface>(m_instance->handle(), window);

        m_physical_device =
            std::make_unique<PhysicalDevice>(m_instance->handle(), m_surface->handle());

        m_device = std::make_unique<Device>(*m_physical_device);

        m_allocator = std::make_unique<Allocator>(m_instance->handle(),
                                                  m_physical_device->handle(),
                                                  m_device->handle());
    }

    Context::~Context()
    {
        // Make sure everything using the device is already destroyed
        // Renderer should call vkDeviceWaitIdle before this.

        m_device.reset();

        m_physical_device.reset();

        m_surface.reset();
        m_debug_messenger.reset();
        m_instance.reset();
    }

    VkInstance Context::instance_handle() const
    {
        return m_instance ? m_instance->handle() : VK_NULL_HANDLE;
    }

    VkDevice Context::device_handle() const
    {
        return m_device ? m_device->handle() : VK_NULL_HANDLE;
    }

    VkSurfaceKHR Context::surface_handle() const
    {
        return m_surface ? m_surface->handle() : VK_NULL_HANDLE;
    }

    VkQueue Context::graphics_queue() const
    {
        return m_device ? m_device->graphics_queue() : VK_NULL_HANDLE;
    }

    VkQueue Context::present_queue() const
    {
        return m_device ? m_device->present_queue() : VK_NULL_HANDLE;
    }

    QueueFamilyIndices Context::queues() const
    {
        return m_physical_device ? m_physical_device->queues() : QueueFamilyIndices{};
    }

} // namespace ankh
