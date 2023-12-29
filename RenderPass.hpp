#pragma once
#include <vulkan/vulkan_raii.hpp>
#include "Device.hpp"
#include "Surface.hpp"


class RenderPass
{
public:
    RenderPass(Device& device, Surface& surface);
    vk::raii::RenderPass& Get();
private:
    vk::RenderPassCreateInfo GetCreateInfo(Surface& surface);
    vk::raii::RenderPass m_RenderPass;    
};