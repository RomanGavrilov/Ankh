#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <array>
#include <cstdint>
#include <optional>
#include <set>
#include <string>
#include <vector>

namespace ankh
{

    inline constexpr uint32_t kWidth = 800;
    inline constexpr uint32_t kHeight = 600;
    inline constexpr int kMaxFramesInFlight = 2;

#ifndef NDEBUG
    inline constexpr bool kEnableValidation = true;
#else
    inline constexpr bool kEnableValidation = false;
#endif

    struct QueueFamilyIndices
    {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;

        bool isComplete() const
        {
            return graphicsFamily && presentFamily;
        }
    };

    struct SwapChainSupportDetails
    {
        VkSurfaceCapabilitiesKHR capabilities{};
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };

    struct Vertex
    {
        glm::vec2 pos;
        glm::vec3 color;
        glm::vec2 uv;

        static VkVertexInputBindingDescription getBindingDescription();
        static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions();
    };

    struct FrameUBO
    {
        alignas(16) glm::mat4 view;
        alignas(16) glm::mat4 proj;
        alignas(16) glm::vec4 globalAlbedo;
    };

    struct ObjectDataGPU
    {
        alignas(16) glm::mat4 model;
        alignas(16) glm::vec4 albedo;
    };

} // namespace ankh
