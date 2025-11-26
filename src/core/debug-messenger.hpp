#pragma once
#include "utils/types.hpp"

namespace ankh
{

    class DebugMessenger
    {
    public:
        explicit DebugMessenger(VkInstance instance);
        ~DebugMessenger();

        // Returns count of validation errors encountered
        static uint32_t error_count();
        // Returns count of validation warnings encountered
        static uint32_t warning_count();
        // Resets error and warning counters
        static void reset_counters();

    private:
        VkInstance m_instance{};
        VkDebugUtilsMessengerEXT m_messenger{};
    };

} // namespace ankh
