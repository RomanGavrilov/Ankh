#include "core/instance.hpp"
#include "utils/config.hpp"
#include <cstring>
#include <stdexcept>
#include <vector>

namespace ankh
{

    static std::vector<const char *> get_required_extensions()
    {
        uint32_t count = 0;
        const char **glfw_extensions = glfwGetRequiredInstanceExtensions(&count);
        std::vector<const char *> exts(glfw_extensions, glfw_extensions + count);
        if (ankh::config().validation)
        {
            exts.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

        return exts;
    }

    Instance::Instance()
    {
        VkApplicationInfo app{};
        app.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        app.pApplicationName = "Ankh";
        app.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        app.pEngineName = "Ankh";
        app.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        app.apiVersion = VK_API_VERSION_1_3;

        auto extensions = get_required_extensions();

        VkInstanceCreateInfo ci{};
        ci.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        ci.pApplicationInfo = &app;
        ci.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        ci.ppEnabledExtensionNames = extensions.data();

        const char *layers[] = {"VK_LAYER_KHRONOS_validation"};
        if (ankh::config().validation)
        {
            ci.enabledLayerCount = 1;
            ci.ppEnabledLayerNames = layers;
        }

        if (vkCreateInstance(&ci, nullptr, &m_instance) != VK_SUCCESS)
            throw std::runtime_error("failed to create Vulkan instance");
    }

    Instance::~Instance()
    {
        if (m_instance)
            vkDestroyInstance(m_instance, nullptr);
    }

} // namespace ankh
