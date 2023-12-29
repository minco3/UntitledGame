#pragma once

#include "Device.hpp"
#include "Log.hpp"
#include <vulkan/vulkan_raii.hpp>

template <typename T> class Buffer
{
public:
    Buffer(
        Device& device, size_t count, vk::BufferUsageFlags usageFlags,
        vk::MemoryPropertyFlags memoryPropertyFlags,
        vk::raii::DeviceMemory& bufferMemory)
        : m_Buffer(
              device.Get(), {{},
                             sizeof(T) * m_Count,
                             usageFlags,
                             vk::SharingMode::eExclusive,
                             0,
                             nullptr}),
          m_Count(count)
    {

        vk::MemoryRequirements memoryRequirements =
            m_Buffer.getMemoryRequirements();

        uint32_t memoryTypeIndex =
            device.FindMemoryType(memoryRequirements, memoryPropertyFlags);

        vk::MemoryAllocateInfo memoryAllocateInfo(
            memoryRequirements.size, memoryTypeIndex);

        bufferMemory = device.Get().allocateMemory(memoryAllocateInfo);

        m_Buffer.bindMemory(*bufferMemory, 0);
    }
    Buffer(const Buffer&) = delete;
    constexpr size_t size() { return sizeof(T) * m_Count; }
    constexpr vk::raii::Buffer& Get() { return m_Buffer; }

private:
    size_t m_Count;
    vk::raii::Buffer m_Buffer;
};