// src/core/context.hpp
#pragma once

#include "core/debug-messenger.hpp"
#include "core/device.hpp"
#include "core/instance.hpp"
#include "core/physical-device.hpp"
#include "memory/allocator.hpp"
#include "platform/surface.hpp"
#include "utils/gpu-resource-tracker.hpp"
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
        explicit Context(GLFWwindow *window);

        Context(const Context &) = delete;
        Context &operator=(const Context &) = delete;
        Context(Context &&) = delete;
        Context &operator=(Context &&) = delete;

        Instance &instance();
        const Instance &instance() const;

        DebugMessenger *debug();
        const DebugMessenger *debug() const;

        PhysicalDevice &physical_device();
        const PhysicalDevice &physical_device() const;

        Device &device();
        const Device &device() const;

        Surface &surface();
        const Surface &surface() const;

        Allocator &allocator();
        const Allocator &allocator() const;

        VkInstance instance_handle() const;
        VkDevice device_handle() const;
        VkSurfaceKHR surface_handle() const;

        VkQueue graphics_queue() const;
        VkQueue present_queue() const;

        QueueFamilyIndices queues() const;

#ifndef NDEBUG
        GpuResourceTracker &gpu_tracker()
        {
            return m_gpu_tracker;
        }

        const GpuResourceTracker &gpu_tracker() const
        {
            return m_gpu_tracker;
        }
#endif

      private:
#ifndef NDEBUG
        GpuResourceTracker m_gpu_tracker;
#endif

        std::unique_ptr<Instance> m_instance;
        std::unique_ptr<DebugMessenger> m_debug_messenger;
        std::unique_ptr<Surface> m_surface;
        std::unique_ptr<PhysicalDevice> m_physical_device;
        std::unique_ptr<Device> m_device;
        std::unique_ptr<Allocator> m_allocator;
    };

} // namespace ankh
