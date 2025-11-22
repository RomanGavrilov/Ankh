#pragma once
#include <string>
#include "utils/Types.hpp"

namespace ankh
{

    class ShaderModule
    {
    public:
        explicit ShaderModule(const std::string &path);
        ~ShaderModule();

        VkShaderModule handle() const;
    };

} // namespace ankh