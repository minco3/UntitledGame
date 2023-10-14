#pragma once

#include <vector>
#include <vulkan/vulkan_raii.hpp>

class DeviceQueue
{
public:
    DeviceQueue(const std::vector<float>& priorities);

    std::vector<float> m_Priorities;
    vk::Bool32 m_PresentationSupported;
};