#include "shaders/shader-module.hpp"
#include "utils/file-io.hpp"
#include <stdexcept>
#include <utils/logging.hpp>

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

        ANKH_VK_CHECK(vkCreateShaderModule(m_device, &ci, nullptr, &m_module));
    }

    ShaderModule::~ShaderModule()
    {
        if (m_module)
            vkDestroyShaderModule(m_device, m_module, nullptr);
    }

} // namespace ankh
