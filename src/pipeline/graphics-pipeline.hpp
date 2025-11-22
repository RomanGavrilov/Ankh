#pragma once
#include "utils/Types.hpp"

namespace ankh
{

    class GraphicsPipeline
    {
    public:
        GraphicsPipeline();
        ~GraphicsPipeline();

        VkPipeline handle() const;
    };

} // namespace ankh