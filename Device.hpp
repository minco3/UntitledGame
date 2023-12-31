#pragma once
#include "DeviceQueue.hpp"
#include "Log.hpp"
#include "Surface.hpp"
#include <vulkan/vulkan_raii.hpp>
#include <vector>

class Device
{
public:
    Device(VulkanInstance& instance, Surface& surface);
    
    // std::vector<vk::raii::PhysicalDevice>
    // GetPhysicalDevices(vk::raii::Instance& instance);
    // VkPhysicalDevice ChoosePhysicalDevice(vk::raii::Instance& instance);
    std::vector<vk::DeviceQueueCreateInfo>
    GetDeviceQueueCreateInfos(Surface& surface);
    std::vector<const char*> GetDeviceExtentionNames();
    vk::raii::Device CreateDevice(Surface& surface);

    vk::raii::Device& Get();
    uint32_t FindMemoryType(
        vk::MemoryRequirements memoryRequirements,
        vk::MemoryPropertyFlags memoryPropertyFlags);

    vk::SurfaceCapabilitiesKHR GetSurfaceCapabilities(vk::raii::SurfaceKHR& device);
    std::vector<vk::SurfaceFormatKHR> GetCompatableSurfaceFormats(vk::raii::SurfaceKHR& surface);

private:
    std::vector<DeviceQueue> m_DeviceQueues;
    vk::raii::PhysicalDevice m_PhysicalDevice;
    vk::raii::Device m_Device;
};