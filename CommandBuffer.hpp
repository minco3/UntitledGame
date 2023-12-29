#pragma once

#include "Device.hpp"

#include <vulkan/vulkan_raii.hpp>

class CommandBuffer
{
public:
    CommandBuffer(Device& device);
private:
    vk::raii::CommandBuffer m_CommandBuffer;
};