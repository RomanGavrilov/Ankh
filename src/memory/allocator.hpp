#pragma once

#include "utils/types.hpp"
#include <vk_mem_alloc.h>

namespace ankh
{
    class Allocator
    {
      public:
        Allocator(VkInstance instance, VkPhysicalDevice phys, VkDevice device);
        ~Allocator();

        Allocator(const Allocator &) = delete;
        Allocator &operator=(const Allocator &) = delete;
        Allocator(Allocator &&) noexcept = delete;
        Allocator &operator=(Allocator &&) noexcept = delete;

        VmaAllocator handle() const
        {
            return m_allocator;
        }

      private:
        VmaAllocator m_allocator{VK_NULL_HANDLE};
    };
} // namespace ankh
