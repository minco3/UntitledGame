#pragma once

#include "Device.hpp"
#include "Log.hpp"
#include <span>
#include <vulkan/vulkan_raii.hpp>

template <typename T> class Buffer
{
public:
    Buffer(
        Device& device, size_t count,
        vk::BufferUsageFlags usageFlags, vk::MemoryPropertyFlags
            memoryPropertyFlags = vk::MemoryPropertyFlagBits::eHostCoherent |
                                  vk::MemoryPropertyFlagBits::eHostVisible)
        : m_Count(count), m_Buffer(
                              device.Get(), {{},
                                             sizeof(T) * m_Count,
                                             usageFlags,
                                             vk::SharingMode::eExclusive,
                                             0,
                                             nullptr}),
          m_DeviceMemory(
              device.Get(), GetMemoryAllocateInfo(device, memoryPropertyFlags)),
          m_Memory(nullptr)

    {
        m_Buffer.bindMemory(*m_DeviceMemory, 0);
    }
    Buffer(const Buffer&) = delete;
    Buffer(Buffer&&) = default;
    ~Buffer()
    {
        if (m_Memory)
        {
            Unmap();
        }
    }
    constexpr size_t size() { return sizeof(T) * m_Count; }
    constexpr vk::raii::Buffer& Get() { return m_Buffer; }
    constexpr vk::raii::DeviceMemory& GetDeviceMemory()
    {
        return m_DeviceMemory;
    }
    std::span<T> GetMemory()
    {
        if (!m_Memory)
        {
            Map();
        }
        return std::span<T>(static_cast<T*>(m_Memory), m_Count);
    }

    vk::MemoryAllocateInfo GetMemoryAllocateInfo(
        Device& device, vk::MemoryPropertyFlags memoryPropertyFlags)
    {
        vk::MemoryRequirements memoryRequirements =
            m_Buffer.getMemoryRequirements();

        uint32_t memoryTypeIndex =
            device.FindMemoryType(memoryRequirements, memoryPropertyFlags);

        return vk::MemoryAllocateInfo(memoryRequirements.size, memoryTypeIndex);
    }

private:
    void Map() { m_Memory = m_DeviceMemory.mapMemory(0, VK_WHOLE_SIZE, {}); }
    void Unmap()
    {
        m_DeviceMemory.unmapMemory();
        m_Memory = nullptr;
    }

private:
    size_t m_Count;
    vk::raii::Buffer m_Buffer;
    vk::raii::DeviceMemory m_DeviceMemory;
    void* m_Memory; // maybe move away from void* in the future?
};