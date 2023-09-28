#pragma once

#include "Window.hpp"
#include <vector>
#include <vulkan/vulkan.h>

class Video
{
public:
    Video();
    ~Video();

private:
    void CreateInstance();
    void CreateDevice();
    std::vector<const char*> GetExtensionNames();
    std::vector<VkExtensionProperties> GetInstanceSupportedExtensions();
    std::vector<VkPhysicalDevice> GetPhysicalDevices();
    std::vector<VkDeviceQueueCreateInfo>
    GetDeviceQueueCreateInfos(VkPhysicalDevice device);

    VkInstance m_Instance;
    Window m_Window;
    VkSurfaceKHR m_Surface;
    VkDevice m_Device;
};