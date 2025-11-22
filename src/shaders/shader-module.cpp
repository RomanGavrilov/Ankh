#include "shaders/shader-module.hpp"
#include "utils/file-io.hpp"
#include <stdexcept>

namespace ankh
{

    ShaderModule::ShaderModule(VkDevice device, const std::string &path)
        : m_device(device)
    {

        auto code = read_binary(path);

        VkShaderModuleCreateInfo ci{};
        ci.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        ci.codeSize = code.size();
        ci.pCode = reinterpret_cast<const uint32_t *>(code.data());

        if (vkCreateShaderModule(m_device, &ci, nullptr, &m_module) != VK_SUCCESS)
            throw std::runtime_error("failed to create shader module: " + path);
    }

    ShaderModule::~ShaderModule()
    {
        if (m_module)
            vkDestroyShaderModule(m_device, m_module, nullptr);
    }

} // namespace ankh
