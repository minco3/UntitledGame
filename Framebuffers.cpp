#include "Framebuffers.hpp"

Framebuffers::Framebuffers(
    Swapchain& swapchain, RenderPass& renderPass, Device& device)
{
    vk::Extent2D swapchainExtent = swapchain.GetExtent();
    std::vector<vk::raii::ImageView>& swapchainImageViews =
        swapchain.GetImageViews();
        
    m_Framebuffers.reserve(swapchainImageViews.size());
    for (auto& imageView : swapchainImageViews)
    {
        vk::FramebufferCreateInfo framebufferCreateInfo(
            {}, *renderPass.Get(), *imageView, swapchainExtent.width,
            swapchainExtent.height, 1);

        m_Framebuffers.emplace_back(*device.Get(), framebufferCreateInfo);
    }

    // for shits and giggles
    // std::ranges::transform(
    //     swapchainImageViews, std::back_inserter(m_Framebuffers),
    //     [&](vk::raii::ImageView& imageView)
    //     {
    //         vk::FramebufferCreateInfo framebufferCreateInfo(
    //             {}, *renderPass.Get(), *imageView, swapchainExtent.width,
    //             swapchainExtent.height, 1);
    //         return vk::raii::Framebuffer(device.Get(), framebufferCreateInfo);
    //     });
}

vk::raii::Framebuffer& Framebuffers::operator[](size_t index)
{
    return m_Framebuffers.at(index);
}