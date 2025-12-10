#include "types.hpp"

namespace ankh
{

    VkVertexInputBindingDescription Vertex::getBindingDescription()
    {
        VkVertexInputBindingDescription binding{};
        binding.binding = 0;
        binding.stride = sizeof(Vertex);
        binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        return binding;
    }

    std::array<VkVertexInputAttributeDescription, 4> Vertex::getAttributeDescriptions()
    {
        /*******************************************************************************
        *
        *   location is the shader input location number for this attribute.

        *   binding is the binding number which this attribute takes its data from.

        *   format is the size and type of the vertex attribute data.

        *   offset is a byte offset of this attribute relative to the start of an element in the
        *   vertex input binding.
        *
        ******************************************************************************/

        std::array<VkVertexInputAttributeDescription, 4> attrs{};

        attrs[0].binding = 0;
        attrs[0].location = 0;
        attrs[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attrs[0].offset = offsetof(Vertex, pos);

        attrs[1].binding = 0;
        attrs[1].location = 1;
        attrs[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attrs[1].offset = offsetof(Vertex, normal);

        attrs[2].binding = 0;
        attrs[2].location = 2;
        attrs[2].format = VK_FORMAT_R32G32B32_SFLOAT;
        attrs[2].offset = offsetof(Vertex, color);

        attrs[3].binding = 0;
        attrs[3].location = 3;
        attrs[3].format = VK_FORMAT_R32G32_SFLOAT;
        attrs[3].offset = offsetof(Vertex, uv);

        return attrs;
    }

} // namespace ankh
