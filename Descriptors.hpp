#pragma once
#include "Device.hpp"
#include "Buffer.hpp"
#include "UniformBuffer.hpp"
#include <vector>
#include <vulkan/vulkan_raii.hpp>

class Descriptors
{
public:
    Descriptors(
        Device& device,
        std::vector<Buffer<UniformBufferObject>>& uniformBuffers,
        size_t imageCount);
    constexpr vk::raii::DescriptorSetLayout& GetLayout()
    {
        return m_DescriptorSetLayout;
    }
    constexpr vk::raii::DescriptorPool& GetPool() { return m_DescriptorPool; }
    constexpr vk::raii::DescriptorSets& GetSets() { return m_DescriptorSets; }

private:
    vk::DescriptorPoolCreateInfo GetDescriptorPoolCreateInfo(size_t imageCount);
    vk::DescriptorSetLayoutCreateInfo GetDescriptorSetLayoutCreateInfo();

private:
    vk::raii::DescriptorPool m_DescriptorPool;
    vk::raii::DescriptorSetLayout m_DescriptorSetLayout;
    vk::raii::DescriptorSets m_DescriptorSets;

    // probably doesnt need to be a member variable
    std::vector<vk::DescriptorSetLayoutBinding> m_DescriptorSetLayoutBindings;
};