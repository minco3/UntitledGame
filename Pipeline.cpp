#include "Pipeline.hpp"
#include "Vertex.hpp"

GraphicsPipeline::GraphicsPipeline(
    Device& device, vk::raii::RenderPass& renderPass, Surface& surface)
    : m_DescriptorSetLayout(device.Get(), GetDescriptorSetLayoutCreateInfo()),
      m_PipelineLayout(
          device.Get(), vk::PipelineLayoutCreateInfo({}, *m_DescriptorSetLayout)),
      m_Pipeline(device.Get(), nullptr, GetPipelineCreateInfo(device.Get(), renderPass, surface.surfaceCapabilities))
{
}

vk::raii::Pipeline& GraphicsPipeline::Get() { return m_Pipeline; }
vk::raii::PipelineLayout& GraphicsPipeline::GetLayout()
{
    return m_PipelineLayout;
}

vk::GraphicsPipelineCreateInfo GraphicsPipeline::GetPipelineCreateInfo(
    vk::raii::Device& device,
    vk::raii::RenderPass& renderPass,
    vk::SurfaceCapabilitiesKHR surfaceCapabilities)
{

    m_Shaders = LoadShaders(device);
    std::vector<vk::PipelineShaderStageCreateInfo> shaderStages =
        CreateShaderStage();

    vk::VertexInputBindingDescription bindingDescription(
        0, sizeof(Vertex), vk::VertexInputRate::eVertex);

    std::array<vk::VertexInputAttributeDescription, 2> attributeDescription = {
        {{0, 0, vk::Format::eR32G32Sfloat, offsetof(Vertex, pos)},
         {1, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, color)}}};

    vk::PipelineVertexInputStateCreateInfo vertexInputState(
        {}, bindingDescription, attributeDescription);

    vk::PipelineInputAssemblyStateCreateInfo inputAssemblyState(
        {}, vk::PrimitiveTopology::eTriangleList, {});

    vk::Viewport viewport(
        0.0f, 0.0f, surfaceCapabilities.currentExtent.width,
        surfaceCapabilities.currentExtent.height, 0.0f, 1.0f);
    vk::Rect2D scissor({}, surfaceCapabilities.currentExtent);

    vk::PipelineViewportStateCreateInfo viewportState({}, viewport, scissor);

    vk::PipelineRasterizationStateCreateInfo rasteriztionState{};
    rasteriztionState.setCullMode(vk::CullModeFlagBits::eBack);

    vk::PipelineMultisampleStateCreateInfo multisampleState{};

    vk::StencilOpState frontStencil{};
    vk::StencilOpState backStencil{};

    vk::PipelineDepthStencilStateCreateInfo depthStencilState{};
    depthStencilState.setDepthCompareOp(vk::CompareOp::eLess);
    depthStencilState.setMaxDepthBounds(100.0f);

    vk::PipelineColorBlendStateCreateInfo colorBlendState{};

    vk::GraphicsPipelineCreateInfo graphicsPipelineCreateInfo(
        {}, shaderStages, &vertexInputState, &inputAssemblyState, {},
        &viewportState, &rasteriztionState, &multisampleState, &depthStencilState,
        &colorBlendState, {}, *m_PipelineLayout, *renderPass);

    return graphicsPipelineCreateInfo;
}

vk::DescriptorSetLayoutCreateInfo
GraphicsPipeline::GetDescriptorSetLayoutCreateInfo()
{
    CreateDescriptorSetLayoutBindings();

    return vk::DescriptorSetLayoutCreateInfo(
        vk::DescriptorSetLayoutCreateFlags(), m_DescriptorSetLayoutBindings);
}

std::vector<vk::PipelineShaderStageCreateInfo>
GraphicsPipeline::CreateShaderStage()
{
    vk::PipelineShaderStageCreateInfo vertShaderStageCreateInfo(
        vk::PipelineShaderStageCreateFlags(), vk::ShaderStageFlagBits::eVertex,
        *m_Shaders.front().vertShaderModule, "main");

    vk::PipelineShaderStageCreateInfo fragShaderStageCreateInfo(
        vk::PipelineShaderStageCreateFlags(),
        vk::ShaderStageFlagBits::eFragment, *m_Shaders.front().fragShaderModule,
        "main");

    std::vector<vk::PipelineShaderStageCreateInfo> shaderStages = {
        vertShaderStageCreateInfo, fragShaderStageCreateInfo};

    return shaderStages;
}

void GraphicsPipeline::CreateDescriptorSetLayoutBindings()
{
    vk::DescriptorSetLayoutBinding descriptorSetLayoutBinding(
        0, vk::DescriptorType::eUniformBuffer, 1,
        vk::ShaderStageFlagBits::eVertex);

    m_DescriptorSetLayoutBindings.push_back(descriptorSetLayoutBinding);
}
