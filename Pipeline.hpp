#pragma once

#include "Descriptors.hpp"
#include "Device.hpp"
#include "RenderPass.hpp"
#include "Shader.hpp"
#include "Surface.hpp"

#include <filesystem>
#include <map>
#include <memory>
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
    bool NeedsUpdate() const;

    void Recreate(
        Device& device, const std::string& shaderName,
        std::filesystem::file_time_type lastModified, RenderPass& renderPass,
        Surface& surface);

    void UpdatePipeline(Device& device);

private:
    vk::raii::Pipeline
    CreatePipeline(Device& device, RenderPass& renderPass, Surface& surface);
    vk::raii::PipelineLayout
    CreatePipelineLayout(Device& device, Descriptors& descriptors);

    vk::DescriptorSetLayoutCreateInfo GetDescriptorSetLayoutCreateInfo();
    void CreateDescriptorSetLayoutBindings();

    std::map<std::string, vk::raii::ShaderModule> m_Shaders; // sacrilidge
    vk::raii::PipelineLayout m_PipelineLayout;
    vk::raii::Pipeline m_Pipeline;
    std::unique_ptr<vk::raii::Pipeline> m_AltPipeline;
    std::filesystem::file_time_type m_LastModified;
};