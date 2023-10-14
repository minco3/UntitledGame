#pragma once
#include "DeviceQueue.hpp"
#include "Log.hpp"
#include "Surface.hpp"
#include "vulkan/vulkan_raii.hpp"
#include <vector>

class Device
{
public:
    Device(vk::raii::Instance& instance, Surface surface);
    std::vector<vk::raii::PhysicalDevice>
    GetPhysicalDevices(vk::raii::Instance& instance);
    std::vector<vk::DeviceQueueCreateInfo>
    GetDeviceQueueCreateInfos(Surface surface);
    std::vector<const char*> GetDeviceExtentionNames();
    vk::raii::Device& operator()();
    uint32_t FindMemoryType(
        vk::MemoryRequirements memoryRequirements,
        vk::MemoryPropertyFlags memoryPropertyFlags)
    {

        vk::PhysicalDeviceMemoryProperties memoryProperties =
            m_PhysicalDevice.getMemoryProperties();

        uint32_t memoryTypeIndex;

        LogDebug(fmt::format(
            "TypeFilter: {:#b}", memoryRequirements.memoryTypeBits));

        LogDebug(fmt::format("Available Memory Types:"));
        for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++)
        {
            LogDebug(fmt::format(
                "\t{:#b}", static_cast<uint32_t>(
                               memoryProperties.memoryTypes[i].propertyFlags)));
        }

        for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++)
        {
            if ((memoryRequirements.memoryTypeBits & (1 << i)) &&
                (memoryProperties.memoryTypes[i].propertyFlags &
                 memoryPropertyFlags) == memoryPropertyFlags)
            {
                memoryTypeIndex = i;
                break;
            }
            if (i == memoryProperties.memoryTypeCount - 1)
            {
                LogError("ERROR: failed to find suitable memory type\n");
            }
        }
        return memoryTypeIndex;
    }
    friend void Surface::GetSurfaceCapabilities(Device device);
    friend const std::vector<VkSurfaceFormatKHR>
    Surface::GetCompatableSurfaceFormats(Device device);

private:
    vk::raii::Device m_Device;
    vk::raii::PhysicalDevice m_PhysicalDevice;
    std::vector<DeviceQueue> m_DeviceQueues;
};