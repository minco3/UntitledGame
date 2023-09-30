#pragma once

#include "DeviceQueue.hpp"
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
    std::vector<VkDeviceQueueCreateInfo> GetDeviceQueueCreateInfos(VkPhysicalDevice device);
    std::vector<const char*>GetDeviceExtentionNames(VkPhysicalDevice device);
    bool CheckDeviceSupportsPresentation(VkPhysicalDevice device);

    std::vector<DeviceQueue> m_DeviceQueues;
    VkInstance m_Instance;
    Window m_Window;
    VkSurfaceKHR m_Surface;
    VkDevice m_Device;
};