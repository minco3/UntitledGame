#pragma once

#include "Device.hpp"
#include "Shader.hpp"
#include "Surface.hpp"

#include <vulkan/vulkan_raii.hpp>

class GraphicsPipeline
{
public:
    GraphicsPipeline(
        Device& device, vk::raii::RenderPass& renderPass, Surface& surface);
    std::vector<vk::PipelineShaderStageCreateInfo> CreateShaderStage();
    void CreateRenderPass();
    void CreateGraphicsPipeline();

    vk::raii::Pipeline& Get();
    vk::raii::PipelineLayout& GetLayout();

private:
    vk::GraphicsPipelineCreateInfo GetPipelineCreateInfo(
        vk::raii::Device& device, vk::raii::RenderPass& renderPass,
        vk::SurfaceCapabilitiesKHR surfaceCapabilities);

    vk::DescriptorSetLayoutCreateInfo GetDescriptorSetLayoutCreateInfo();
    void CreateDescriptorSetLayoutBindings();

    std::vector<Shader> m_Shaders;
    std::vector<vk::DescriptorSetLayoutBinding> m_DescriptorSetLayoutBindings;
    vk::raii::DescriptorSetLayout m_DescriptorSetLayout;
    vk::raii::PipelineLayout m_PipelineLayout;
    vk::raii::Pipeline m_Pipeline;
};