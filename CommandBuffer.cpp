#include "CommandBuffer.hpp"

CommandBuffer::CommandBuffer(Device& device)
:m_CommandBuffer(device.Get(), commandBuffer, commandPool)
{}