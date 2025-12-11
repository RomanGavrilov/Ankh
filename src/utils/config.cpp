// src/utils/config.cpp
#include "utils/config.hpp"
#include "utils/logging.hpp"

namespace ankh
{
    Config &config()
    {
        static Config cfg{};

#ifndef NDEBUG
        log::SetLevel(log::Level::Debug);
#else
        log::SetLevel(log::Level::Warn);
#endif

        return cfg;
    }

} // namespace ankh
