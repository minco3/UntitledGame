#include "Descriptors.hpp"

// TODO: maxSets needs to be revisited later

Descriptors::Descriptors(
    Device& device, std::vector<Buffer<UniformBufferObject>>& uniformBuffers,
    size_t imageCount, size_t setsPerImage)
    : m_DescriptorPool(CreateDescriptorPool(device, imageCount*setsPerImage)),
      m_DescriptorSetLayouts(CreateDescriptorSetLayouts(device, imageCount)),
      m_DescriptorSets(CreateDescriptorSets(device))
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

vk::raii::DescriptorPool
Descriptors::CreateDescriptorPool(Device& device, size_t maxSets)
{
    vk::DescriptorPoolSize discriptorPoolSize(
        vk::DescriptorType::eUniformBuffer, maxSets);
    vk::DescriptorPoolCreateInfo createInfo(
        vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet, maxSets,
        discriptorPoolSize);
    return device.Get().createDescriptorPool(createInfo);
}

std::vector<vk::raii::DescriptorSetLayout>
Descriptors::CreateDescriptorSetLayouts(Device& device, size_t imageCount)
{
    vk::DescriptorSetLayoutBinding descriptorSetLayoutBinding(
        0, vk::DescriptorType::eUniformBuffer, 1,
        vk::ShaderStageFlagBits::eVertex);

    m_DescriptorSetLayoutBindings.push_back(descriptorSetLayoutBinding);

    vk::DescriptorSetLayoutCreateInfo createInfo(
        {}, m_DescriptorSetLayoutBindings);

    std::vector<vk::raii::DescriptorSetLayout> descriptorSetLayouts;

    for (size_t i = 0; i < imageCount; i++)
    {
        descriptorSetLayouts.emplace_back(
            device.Get().createDescriptorSetLayout(createInfo));
    }
    return descriptorSetLayouts;
}

vk::raii::DescriptorSets Descriptors::CreateDescriptorSets(Device& device)
{
    std::vector<vk::DescriptorSetLayout> setLayouts;
    for (auto& layout : m_DescriptorSetLayouts)
    {
        setLayouts.push_back(*layout);
    }

    vk::DescriptorSetAllocateInfo allocInfo(*m_DescriptorPool, setLayouts);
    return vk::raii::DescriptorSets(device.Get(), allocInfo);
}