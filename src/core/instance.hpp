#pragma once
#include <vector>
#include <string>
#include "utils/Types.hpp"

namespace ankh
{

    class Instance
    {
    public:
        Instance();
        ~Instance();

        VkInstance handle() const;
    };

} // namespace ankh