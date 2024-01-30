#include "CommandBuffer.hpp"

CommandBuffer::CommandBuffer(
    Device& device, uint32_t queueIndex, size_t bufferCount)
    : m_CommandPool(
          device.Get(),
          vk::CommandPoolCreateInfo(
              vk::CommandPoolCreateFlagBits::eResetCommandBuffer, queueIndex)),
      m_CommandBuffers(
          device.Get(),
          vk::CommandBufferAllocateInfo(
              *m_CommandPool, vk::CommandBufferLevel::ePrimary, bufferCount))
{
}

vk::raii::CommandBuffers& CommandBuffer::Get()
{
    return m_CommandBuffers;
}

vk::raii::CommandBuffer& CommandBuffer::operator[](size_t index)
{
    return m_CommandBuffers.at(index);
}
