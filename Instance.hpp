#pragma once
#include "Window.hpp"
#include <vector>
#include <vulkan/vulkan_core.h>

class VulkanInstance
{
public:
    VulkanInstance(Window window);
    constexpr VkInstance& operator()() { return m_Instance; }

private:
    std::vector<const char*> GetExtensionNames(const Window& window);
    VkInstanceCreateFlags
    GetInstanceCreateFlags(const std::vector<const char*>& extentionNames);
    std::vector<VkExtensionProperties> GetInstanceSupportedExtensions();

    VkInstance m_Instance;
};