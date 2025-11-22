#pragma once
#include "utils/types.hpp"

namespace ankh
{

    class Instance
    {
    public:
        Instance();
        ~Instance();

        VkInstance handle() const { return m_instance; }

    private:
        VkInstance m_instance{};
    };

} // namespace ankh
