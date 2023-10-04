#pragma once

#include <vector>
#include <vulkan/vulkan_core.h>

class DeviceQueue
{
public:
    DeviceQueue(const std::vector<float>& priorities);

    std::vector<float> m_Priorities;
    VkBool32 m_PresentationSupported;
};