#pragma once
#include "utils/Types.hpp"

namespace ankh
{

    class Image
    {
    public:
        Image();
        ~Image();

        VkImage handle() const;
    };

} // namespace ankh