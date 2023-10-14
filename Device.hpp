#pragma once
#include "DeviceQueue.hpp"
#include "Log.hpp"
#include "Surface.hpp"
#include "vulkan/vulkan_raii.hpp"
#include <vector>

class Device
{
public:
    Device(vk::raii::Instance& instance, Surface& surface);
    std::vector<vk::raii::PhysicalDevice>
    GetPhysicalDevices(vk::raii::Instance& instance);
    VkPhysicalDevice ChoosePhysicalDevice(vk::raii::Instance& instance);
    std::vector<vk::DeviceQueueCreateInfo>
    GetDeviceQueueCreateInfos(Surface& surface);
    std::vector<const char*> GetDeviceExtentionNames();
    vk::DeviceCreateInfo GetDeviceCreateInfo(Surface& surface);

    vk::raii::Device& operator()();
    uint32_t FindMemoryType(
        vk::MemoryRequirements memoryRequirements,
        vk::MemoryPropertyFlags memoryPropertyFlags);

    friend void Surface::GetSurfaceCapabilities(Device device);
    friend const std::vector<vk::SurfaceFormatKHR>
    Surface::GetCompatableSurfaceFormats(Device device);

private:
    vk::raii::PhysicalDevice m_PhysicalDevice;
    vk::raii::Device m_Device;
    std::vector<DeviceQueue> m_DeviceQueues;
};