#pragma once
#include "utils/Types.hpp"
#include <cstddef>

namespace ankh
{

    class Buffer
    {
    public:
        Buffer();
        ~Buffer();

        VkBuffer handle() const;
        std::size_t size() const;
    };

} // namespace ankh