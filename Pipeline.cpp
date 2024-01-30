#include "Pipeline.hpp"
#include "Vertex.hpp"
#include <algorithm>
#include <chrono>

GraphicsPipeline::GraphicsPipeline(
    Device& device, RenderPass& renderPass, Surface& surface,
    Descriptors& descriptors)
    : m_Shaders(LoadShaders(device.Get())),
      m_PipelineLayout(CreatePipelineLayout(device, descriptors)),
      m_Pipeline(CreatePipeline(device, renderPass, surface)),
      m_LastModified(std::chrono::file_clock::now().time_since_epoch())
{
}

vk::raii::Pipeline& GraphicsPipeline::Get() { return m_Pipeline; }
vk::raii::PipelineLayout& GraphicsPipeline::GetLayout()
{
    return m_PipelineLayout;
}

bool GraphicsPipeline::NeedsUpdate() const
{
    return static_cast<bool>(m_AltPipeline);
}

void GraphicsPipeline::UpdatePipeline(Device& device)
{
    if (!m_AltPipeline)
    {
        LogWarning("No alt pipeline!");
        return;
    }
    device.Get().waitIdle();
    m_Pipeline.swap(*m_AltPipeline.get());
    m_AltPipeline.reset();
}

void GraphicsPipeline::Recreate(
    Device& device, const std::string& shaderName,
    std::filesystem::file_time_type lastModified, RenderPass& renderPass,
    Surface& surface)
{
    // pipeline is newer than shader file update
    if (m_LastModified.time_since_epoch() > lastModified.time_since_epoch())
    {
        return;
    }
    std::optional<vk::raii::ShaderModule> shader =
        CompileShader(device.Get(), shaderName);
    if (!shader.has_value())
    {
        return;
    }
    m_LastModified = lastModified;
    auto it = m_Shaders.find(shaderName);
    if (it != m_Shaders.end())
    {
        std::swap(it->second, shader.value()); // TODO: make more dynamic
    }
    else
    {
        m_Shaders.emplace(shaderName, std::move(shader.value()));
    }
    m_AltPipeline = std::make_unique<vk::raii::Pipeline>(
        CreatePipeline(device, renderPass, surface));
}

vk::raii::Pipeline GraphicsPipeline::CreatePipeline(
    Device& device, RenderPass& renderPass, Surface& surface)
{
    std::vector<vk::PipelineShaderStageCreateInfo> shaderStages =
        CreateShaderStage();

    vk::VertexInputBindingDescription bindingDescription(
        0, sizeof(Vertex), vk::VertexInputRate::eVertex);

    std::array<vk::VertexInputAttributeDescription, 2> attributeDescription = {
        {{0, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, pos)},
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

    vk::PipelineRasterizationStateCreateInfo rasterizationState{};
    rasterizationState.setCullMode(vk::CullModeFlagBits::eBack);
    rasterizationState.setLineWidth(1.0f);

    vk::PipelineMultisampleStateCreateInfo multisampleState{};

    vk::StencilOpState frontStencil{};
    vk::StencilOpState backStencil{};

    vk::PipelineDepthStencilStateCreateInfo depthStencilState{};
    depthStencilState.setDepthCompareOp(vk::CompareOp::eLess);
    depthStencilState.setMaxDepthBounds(100.0f);
    depthStencilState.setDepthWriteEnable(VK_TRUE);

    vk::PipelineColorBlendAttachmentState colorAttachment{};
    colorAttachment.setColorWriteMask(
        vk::FlagTraits<vk::ColorComponentFlagBits>::allFlags);
    vk::PipelineColorBlendStateCreateInfo colorBlendState(
        {}, VK_FALSE, vk::LogicOp::eClear, colorAttachment);

    vk::GraphicsPipelineCreateInfo graphicsPipelineCreateInfo(
        {}, shaderStages, &vertexInputState, &inputAssemblyState, {},
        &viewportState, &rasterizationState, &multisampleState,
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
        *m_Shaders.at("shader.vert"), "main");

    vk::PipelineShaderStageCreateInfo fragShaderStageCreateInfo(
        vk::PipelineShaderStageCreateFlags(),
        vk::ShaderStageFlagBits::eFragment, *m_Shaders.at("shader.frag"),
        "main");

    std::vector<vk::PipelineShaderStageCreateInfo> shaderStages = {
        vertShaderStageCreateInfo, fragShaderStageCreateInfo};

    return shaderStages;
}