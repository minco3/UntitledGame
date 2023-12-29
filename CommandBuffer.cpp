#include "CommandBuffer.hpp"

CommandBuffer::CommandBuffer(Device& device, uint32_t queueIndex)
    : m_CommandPool(device.Get(), vk::CommandPoolCreateInfo({}, queueIndex)),
      m_CommandBuffers(
          device.Get(), vk::CommandBufferAllocateInfo(
                            *m_CommandPool, vk::CommandBufferLevel::ePrimary, 1))
{
}
