#pragma once
#include "utils/types.hpp"

namespace ankh
{

    class PhysicalDevice;

    class Device
    {
    public:
        explicit Device(const PhysicalDevice &phys);
        ~Device();

        VkDevice handle() const { return m_device; }
        VkQueue graphics_queue() const { return m_graphics_queue; }
        VkQueue present_queue() const { return m_present_queue; }

    private:
        VkDevice m_device{};
        VkQueue m_graphics_queue{};
        VkQueue m_present_queue{};
    };

} // namespace ankh
