#include "Descriptors.hpp"

Descriptors::Descriptors(
    Device& device, std::vector<Buffer<UniformBufferObject>>& uniformBuffers,
    size_t imageCount)
    : m_DescriptorPool(device.Get(), GetDescriptorPoolCreateInfo(imageCount)),
      m_DescriptorSetLayout(device.Get(), GetDescriptorSetLayoutCreateInfo()),
      m_DescriptorSets(
          device.Get(), vk::DescriptorSetAllocateInfo(
                            *m_DescriptorPool, *m_DescriptorSetLayout))
{
    std::vector<vk::WriteDescriptorSet> descriptorWriteSets;
    for (size_t i = 0; i < imageCount; i++)
    {
        // need a more dynamic solution
        vk::DescriptorBufferInfo bufferInfo(
            *uniformBuffers.at(i).Get(), 0, sizeof(UniformBufferObject));
        descriptorWriteSets.emplace_back(
            *m_DescriptorSets.at(i), 0, 0, vk::DescriptorType::eUniformBuffer,
            nullptr, bufferInfo, nullptr);
    }
    device.Get().updateDescriptorSets(descriptorWriteSets, nullptr);
}

vk::DescriptorPoolCreateInfo
Descriptors::GetDescriptorPoolCreateInfo(size_t imageCount)
{
    vk::DescriptorPoolSize discriptorPoolSize(
        vk::DescriptorType::eUniformBuffer, imageCount);
    return vk::DescriptorPoolCreateInfo({}, 1, discriptorPoolSize);
}

vk::DescriptorSetLayoutCreateInfo
Descriptors::GetDescriptorSetLayoutCreateInfo()
{
    vk::DescriptorSetLayoutBinding descriptorSetLayoutBinding(
        0, vk::DescriptorType::eUniformBuffer, 1,
        vk::ShaderStageFlagBits::eVertex);

    m_DescriptorSetLayoutBindings.push_back(descriptorSetLayoutBinding);

    return vk::DescriptorSetLayoutCreateInfo({}, m_DescriptorSetLayoutBindings);
}