// src/core/context.hpp
#pragma once

#include "core/debug-messenger.hpp"
#include "core/device.hpp"
#include "core/instance.hpp"
#include "core/physical-device.hpp"
#include "memory/allocator.hpp"
#include "platform/surface.hpp"
#include "utils/types.hpp"
#include <memory>

namespace ankh
{

    class Instance;
    class DebugMessenger;
    class PhysicalDevice;
    class Device;
    class Surface;

    class Context
    {
      public:
        // window is only needed to create the VkSurface
        explicit Context(GLFWwindow *window);
        ~Context();

        Context(const Context &) = delete;
        Context &operator=(const Context &) = delete;
        Context(Context &&) = delete;
        Context &operator=(Context &&) = delete;

        Instance &instance()
        {
            return *m_instance;
        }

        DebugMessenger *debug()
        {
            return m_debug_messenger.get();
        }

        PhysicalDevice &physical_device()
        {
            return *m_physical_device;
        }

        Device &device()
        {
            return *m_device;
        }

        Surface &surface()
        {
            return *m_surface;
        }

        Allocator &allocator()
        {
            return *m_allocator;
        }

        const Allocator &allocator() const
        {
            return *m_allocator;
        }

        // Convenience raw handles / info
        VkInstance instance_handle() const;
        VkDevice device_handle() const;
        VkSurfaceKHR surface_handle() const;

        VkQueue graphics_queue() const;
        VkQueue present_queue() const;

        QueueFamilyIndices queues() const;

      private:
        std::unique_ptr<Instance> m_instance;
        std::unique_ptr<DebugMessenger> m_debug_messenger;
        std::unique_ptr<Surface> m_surface;
        std::unique_ptr<PhysicalDevice> m_physical_device;
        std::unique_ptr<Allocator> m_allocator;
        std::unique_ptr<Device> m_device;
    };

} // namespace ankh
