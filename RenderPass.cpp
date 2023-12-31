#include "RenderPass.hpp"

RenderPass::RenderPass(Device& device, Surface& surface)
    : m_RenderPass(CreateRenderPass(device, surface))
{
}

vk::raii::RenderPass& RenderPass::Get()
{
    return m_RenderPass;
}


vk::raii::RenderPass RenderPass::CreateRenderPass(Device& device, Surface& surface)
{
    vk::AttachmentDescription colorAttachment(
        {}, surface.surfaceFormat.format, vk::SampleCountFlagBits::e1,
        vk::AttachmentLoadOp::eLoad, vk::AttachmentStoreOp::eStore,
        vk::AttachmentLoadOp::eDontCare, vk::AttachmentStoreOp::eDontCare,
        vk::ImageLayout::eUndefined, vk::ImageLayout::ePresentSrcKHR);

    vk::AttachmentReference colorAttachmentReference(
        0, vk::ImageLayout::eColorAttachmentOptimal);

    vk::SubpassDescription subpass({}, vk::PipelineBindPoint::eGraphics, colorAttachmentReference);

    vk::RenderPassCreateInfo renderPassCreateInfo({}, colorAttachment, subpass);

    return device.Get().createRenderPass(renderPassCreateInfo);
}
