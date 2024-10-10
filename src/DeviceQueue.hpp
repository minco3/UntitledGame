#pragma once

#include <vector>
#include <vulkan/vulkan_raii.hpp>

class DeviceQueue
{
public:
    DeviceQueue(
        const std::vector<float>& priorities, vk::Bool32 presentationSupported);

    std::vector<float> m_Priorities;
    vk::Bool32 m_PresentationSupported;
};