#pragma once

#include "Device.hpp"

#include <vulkan/vulkan_raii.hpp>

class CommandBuffer
{
public:
    CommandBuffer(Device& device, uint32_t queueIndex);
private:
    vk::raii::CommandPool m_CommandPool;
    vk::raii::CommandBuffers m_CommandBuffers;
};