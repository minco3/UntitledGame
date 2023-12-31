#pragma once

#include "Device.hpp"

#include <vulkan/vulkan_raii.hpp>

class CommandBuffer
{
public:
    CommandBuffer(Device& device, uint32_t queueIndex, size_t bufferCount = 1);

    vk::raii::CommandBuffer& operator[](size_t index);
private:
    vk::raii::CommandPool m_CommandPool;
    vk::raii::CommandBuffers m_CommandBuffers;
};