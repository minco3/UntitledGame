#pragma once

#include "Device.hpp"
#include "Log.hpp"
#include <vulkan/vulkan_core.h>

template <typename T> class Buffer
{
public:
    Buffer(){};
    void Create(
        Device device, size_t count, VkBufferUsageFlags usageFlags,
        VkMemoryPropertyFlags memoryPropertyFlags, VkDeviceMemory& bufferMemory)
    {
        m_Count = count;
        VkResult result;
        VkBufferCreateInfo bufferCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .size = sizeof(T) * m_Count,
            .usage = usageFlags,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE};
        result =
            vkCreateBuffer(device(), &bufferCreateInfo, nullptr, &m_Buffer);
        LogVulkanError("failed to create vertex buffer", result);

        VkMemoryRequirements memoryRequirements;
        vkGetBufferMemoryRequirements(device(), m_Buffer, &memoryRequirements);
        uint32_t memoryTypeIndex =
            device.FindMemoryType(memoryRequirements, memoryPropertyFlags);

        VkMemoryAllocateInfo memoryAllocateInfo = {
            .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
            .allocationSize = memoryRequirements.size,
            .memoryTypeIndex = memoryTypeIndex};

        result = vkAllocateMemory(
            device(), &memoryAllocateInfo, nullptr, &bufferMemory);
        LogVulkanError("failed to allocate buffer memory", result);

        result = vkBindBufferMemory(device(), m_Buffer, bufferMemory, 0);
        LogVulkanError("failed to bind buffer memory", result);
    }
    constexpr size_t size() { return sizeof(T) * m_Count; }
    constexpr VkBuffer& operator()() { return m_Buffer; }

private:
    VkBuffer m_Buffer;
    size_t m_Count;
};