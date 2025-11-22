#pragma once
#include <string>
#include "utils/types.hpp"

namespace ankh
{

    class ShaderModule
    {
    public:
        ShaderModule(VkDevice device, const std::string &path);
        ~ShaderModule();

        VkShaderModule handle() const { return m_module; }

    private:
        VkDevice m_device{};
        VkShaderModule m_module{};
    };

} // namespace ankh
