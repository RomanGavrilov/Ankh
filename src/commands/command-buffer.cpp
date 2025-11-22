// src/commands/command-buffer.cpp
#include "command-buffer.hpp"
namespace ankh
{
    CommandBuffer::CommandBuffer() {}
    CommandBuffer::~CommandBuffer() {}
    VkCommandBuffer CommandBuffer::handle() const { return m_cb; }
}