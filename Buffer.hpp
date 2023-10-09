#pragma once

#include <vulkan/vulkan_core.h>

template <typename T> class Buffer
{
public:
    Buffer(){};
    void Create(
        VkDevice device, VkPhysicalDevice physicalDevice, size_t count,
        VkBufferUsageFlags usageFlags,
        VkMemoryPropertyFlags memoryPropertyFlags, VkDeviceMemory& bufferMemory)
    {
        m_Count = count;
        VkResult result;
        VkBufferCreateInfo bufferCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .size = sizeof(T) * m_Count,
            .usage = usageFlags,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE};
        result = vkCreateBuffer(device, &bufferCreateInfo, nullptr, &m_Buffer);
        LogVulkanError("failed to create vertex buffer", result);

        VkMemoryRequirements memoryRequirements;
        vkGetBufferMemoryRequirements(device, m_Buffer, &memoryRequirements);
        uint32_t memoryTypeIndex = FindMemoryType(
            physicalDevice, memoryRequirements, memoryPropertyFlags);

        VkMemoryAllocateInfo memoryAllocateInfo = {
            .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
            .allocationSize = memoryRequirements.size,
            .memoryTypeIndex = memoryTypeIndex};

        result = vkAllocateMemory(
            device, &memoryAllocateInfo, nullptr, &bufferMemory);
        LogVulkanError("failed to allocate buffer memory", result);

        result = vkBindBufferMemory(device, m_Buffer, bufferMemory, 0);
        LogVulkanError("failed to bind buffer memory", result);
    }
    constexpr size_t size()
    {
        return sizeof(T)*m_Count;
    }
    constexpr VkBuffer& operator()()
    {
        return m_Buffer;
    }

private:
    uint32_t FindMemoryType(
        VkPhysicalDevice physicalDevice,
        VkMemoryRequirements memoryRequirements,
        VkMemoryPropertyFlags memoryPropertyFlags)
    {
        VkPhysicalDeviceMemoryProperties memoryProperties;
        vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memoryProperties);

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
    VkBuffer m_Buffer;
    size_t m_Count;
};