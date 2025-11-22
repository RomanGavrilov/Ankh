#pragma once
#include "utils/Types.hpp"

namespace ankh
{

    class Device
    {
    public:
        Device();
        ~Device();

        VkDevice handle() const;
        VkQueue graphicsQueue() const;
        VkQueue presentQueue() const;
    };

} // namespace ankh