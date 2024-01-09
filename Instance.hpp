#pragma once
#include "Window.hpp"
#include <vector>
#include <vulkan/vulkan_raii.hpp>

class VulkanInstance
{
public:
    VulkanInstance(const Window& window, vk::raii::Context& context);
    vk::raii::Instance& Get() { return m_Instance; }

private:
    vk::raii::Instance
    CreateInstance(const Window& window, vk::raii::Context& context);
    std::vector<const char*> GetExtensionNames(const Window& window);
    vk::InstanceCreateFlags
    GetInstanceCreateFlags(const std::vector<const char*>& extentionNames);

    vk::raii::Instance m_Instance;
};