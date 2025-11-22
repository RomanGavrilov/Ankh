#pragma once
#include "utils/Types.hpp"

namespace ankh
{

    class PipelineLayout
    {
    public:
        PipelineLayout();
        ~PipelineLayout();

        VkPipelineLayout handle() const;
    };

} // namespace ankh