#pragma once
#include "Buffer.hpp"
#include "Device.hpp"
#include "UniformBuffer.hpp"
#include <vector>
#include <vulkan/vulkan_raii.hpp>

class Descriptors
{
public:
    Descriptors(
        Device& device,
        std::vector<Buffer<UniformBufferObject>>& uniformBuffers,
        size_t imageCount, size_t setsPerImage);
    constexpr std::vector<vk::raii::DescriptorSetLayout>& GetLayouts()
    {
        return m_DescriptorSetLayouts;
    }
    constexpr vk::raii::DescriptorPool& GetPool() { return m_DescriptorPool; }
    constexpr vk::raii::DescriptorSets& GetSets() { return m_DescriptorSets; }

private:
    vk::raii::DescriptorPool
    CreateDescriptorPool(Device& device, size_t maxSets);
    std::vector<vk::raii::DescriptorSetLayout>
    CreateDescriptorSetLayouts(Device& device, size_t imageCount);
    vk::raii::DescriptorSets
    CreateDescriptorSets(Device& device);

private:
    vk::raii::DescriptorPool m_DescriptorPool;
    // probably doesnt need to be a member variable
    std::vector<vk::DescriptorSetLayoutBinding> m_DescriptorSetLayoutBindings;
    std::vector<vk::raii::DescriptorSetLayout> m_DescriptorSetLayouts;
    vk::raii::DescriptorSets m_DescriptorSets;

};