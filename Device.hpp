#pragma once
#include "DeviceQueue.hpp"
#include "Log.hpp"
#include "Surface.hpp"
#include "vulkan/vulkan_core.h"
#include <vector>

class Device
{
public:
    Device(VkInstance& instance, Surface surface);
    std::vector<VkPhysicalDevice> GetPhysicalDevices(VkInstance instance);
    std::vector<VkDeviceQueueCreateInfo>
    GetDeviceQueueCreateInfos(Surface surface);
    std::vector<const char*> GetDeviceExtentionNames();
    VkDevice& operator()();
    uint32_t FindMemoryType(
        VkMemoryRequirements memoryRequirements,
        VkMemoryPropertyFlags memoryPropertyFlags)
    {
        VkPhysicalDeviceMemoryProperties memoryProperties;
        vkGetPhysicalDeviceMemoryProperties(
            m_PhysicalDevice, &memoryProperties);

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
    friend void
    Surface::GetSurfaceCapabilities(Device device);
    friend const std::vector<VkSurfaceFormatKHR>
    Surface::GetCompatableSurfaceFormats(Device device);

private:
    VkDevice m_Device;
    VkPhysicalDevice m_PhysicalDevice;
    std::vector<DeviceQueue> m_DeviceQueues;
};