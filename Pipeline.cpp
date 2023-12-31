#include "Pipeline.hpp"
#include "Vertex.hpp"

GraphicsPipeline::GraphicsPipeline(
    Device& device, RenderPass& renderPass, Surface& surface,
    Descriptors& descriptors)
    : m_PipelineLayout(CreatePipelineLayout(device, descriptors)),
      m_Pipeline(CreatePipeline(device, renderPass, surface))
{
}

vk::raii::Pipeline& GraphicsPipeline::Get() { return m_Pipeline; }
vk::raii::PipelineLayout& GraphicsPipeline::GetLayout()
{
    return m_PipelineLayout;
}

vk::raii::Pipeline GraphicsPipeline::CreatePipeline(
    Device& device, RenderPass& renderPass, Surface& surface)
{
    m_Shaders = LoadShaders(device.Get());
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
        0.0f, 0.0f, surface.surfaceCapabilities.currentExtent.width,
        surface.surfaceCapabilities.currentExtent.height, 0.0f, 1.0f);
    vk::Rect2D scissor({}, surface.surfaceCapabilities.currentExtent);

    vk::PipelineViewportStateCreateInfo viewportState({}, viewport, scissor);

    vk::PipelineRasterizationStateCreateInfo rasteriztionState{};
    rasteriztionState.setCullMode(vk::CullModeFlagBits::eBack);
    rasteriztionState.setLineWidth(1.0f);

    vk::PipelineMultisampleStateCreateInfo multisampleState{};

    vk::StencilOpState frontStencil{};
    vk::StencilOpState backStencil{};

    vk::PipelineDepthStencilStateCreateInfo depthStencilState{};
    depthStencilState.setDepthCompareOp(vk::CompareOp::eLess);
    depthStencilState.setMaxDepthBounds(100.0f);

    vk::PipelineColorBlendAttachmentState colorAttachment{};
    vk::PipelineColorBlendStateCreateInfo colorBlendState({}, VK_FALSE, vk::LogicOp::eClear, colorAttachment);

    vk::GraphicsPipelineCreateInfo graphicsPipelineCreateInfo(
        {}, shaderStages, &vertexInputState, &inputAssemblyState, {},
        &viewportState, &rasteriztionState, &multisampleState,
        &depthStencilState, &colorBlendState, {}, *m_PipelineLayout,
        *renderPass.Get());

    return device.Get().createGraphicsPipeline(
        nullptr, graphicsPipelineCreateInfo);
}

vk::raii::PipelineLayout
GraphicsPipeline::CreatePipelineLayout(Device& device, Descriptors& descriptors)
{
    std::vector<vk::DescriptorSetLayout> descriptorSetLayouts;
    for (auto& layout : descriptors.GetLayouts())
    {
        descriptorSetLayouts.push_back(*layout);
    }
    vk::PipelineLayoutCreateInfo createInfo({}, descriptorSetLayouts);
    return device.Get().createPipelineLayout(createInfo);
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