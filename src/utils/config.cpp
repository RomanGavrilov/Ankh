// src/utils/config.cpp
#include "utils/config.hpp"

namespace ankh
{
    Config &config()
    {
        static Config cfg{};
        return cfg;
    }

} // namespace ankh
