#pragma once

#include "Descriptors.hpp"
#include "Device.hpp"
#include "RenderPass.hpp"
#include "Shader.hpp"
#include "Surface.hpp"

#include <vulkan/vulkan_raii.hpp>

class GraphicsPipeline
{
public:
    GraphicsPipeline(
        Device& device, RenderPass& renderPass, Surface& surface,
        Descriptors& descriptors);
    std::vector<vk::PipelineShaderStageCreateInfo> CreateShaderStage();

    vk::raii::Pipeline& Get();
    vk::raii::PipelineLayout& GetLayout();

private:
    vk::raii::Pipeline
    CreatePipeline(Device& device, RenderPass& renderPass, Surface& surface);
    vk::raii::PipelineLayout
    CreatePipelineLayout(Device& device, Descriptors& descriptors);

    vk::DescriptorSetLayoutCreateInfo GetDescriptorSetLayoutCreateInfo();
    void CreateDescriptorSetLayoutBindings();

    std::vector<Shader> m_Shaders;
    vk::raii::PipelineLayout m_PipelineLayout;
    vk::raii::Pipeline m_Pipeline;
};