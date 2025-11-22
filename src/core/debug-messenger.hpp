#pragma once
#include "utils/types.hpp"

namespace ankh
{

    class DebugMessenger
    {
    public:
        explicit DebugMessenger(VkInstance instance);
        ~DebugMessenger();

    private:
        VkInstance m_instance{};
        VkDebugUtilsMessengerEXT m_messenger{};
    };

} // namespace ankh
