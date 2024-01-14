#include "Framebuffers.hpp"
#include <algorithm>  //for_each
#include <functional> //mem_fn

Framebuffers::Framebuffers(
    Swapchain& swapchain, RenderPass& renderPass, Device& device)
    : m_Framebuffers(CreateFramebuffers(swapchain, renderPass, device))
{
}

void Framebuffers::Recreate(
    Swapchain& swapchain, RenderPass& renderPass, Device& device)
{
    // lovely
    std::ranges::for_each(
        m_Framebuffers, std::mem_fn(&vk::raii::Framebuffer::clear));

    m_Framebuffers = CreateFramebuffers(swapchain, renderPass, device);
}

vk::raii::Framebuffer& Framebuffers::operator[](size_t index)
{
    return m_Framebuffers.at(index);
}

std::vector<vk::raii::Framebuffer> Framebuffers::CreateFramebuffers(
    Swapchain& swapchain, RenderPass& renderPass, Device& device)
{

    std::vector<vk::raii::Framebuffer> framebuffers;
    vk::Extent2D swapchainExtent = swapchain.GetExtent();
    std::vector<vk::raii::ImageView>& swapchainImageViews =
        swapchain.GetImageViews();

    framebuffers.reserve(swapchainImageViews.size());
    for (auto& imageView : swapchainImageViews)
    {
        vk::FramebufferCreateInfo framebufferCreateInfo(
            {}, *renderPass.Get(), *imageView, swapchainExtent.width,
            swapchainExtent.height, 1);

        framebuffers.emplace_back(device.Get(), framebufferCreateInfo);
    }

    // for shits and giggles
    // std::ranges::transform(
    //     swapchainImageViews, std::back_inserter(framebuffers),
    //     [&](vk::raii::ImageView& imageView)
    //     {
    //         vk::FramebufferCreateInfo framebufferCreateInfo(
    //             {}, *renderPass.Get(), *imageView, swapchainExtent.width,
    //             swapchainExtent.height, 1);
    //         return vk::raii::Framebuffer(device.Get(),
    //         framebufferCreateInfo);
    //     });

    return framebuffers;
}
