#include "Swapchain.hpp"

Swapchain::Swapchain(Device& device, Surface& surface)
    : m_Swapchain(CreateSwapchain(device, surface)),
      m_SwapchainImageViews(
          CreateSwapchainImageViews(device.Get(), surface.surfaceFormat)),
    m_Extent(surface.surfaceCapabilities.currentExtent)
{
}

void Swapchain::Recreate(Device& device, Surface& surface)
{
    m_Swapchain.release();
    m_Swapchain = CreateSwapchain(device, surface);
    m_SwapchainImageViews = CreateSwapchainImageViews(device.Get(), surface.surfaceFormat);
    m_Extent = surface.surfaceCapabilities.currentExtent;
}

vk::raii::SwapchainKHR
Swapchain::CreateSwapchain(Device& device, Surface& surface)
{
    surface.GetSurfaceCapabilities(device);
    surface.GetSurfaceFormat(device);

    vk::SwapchainCreateInfoKHR createInfo(
        {}, *surface.Get(), surface.surfaceCapabilities.minImageCount + 1,
        surface.surfaceFormat.format, surface.surfaceFormat.colorSpace,
        surface.surfaceCapabilities.currentExtent, 1,
        vk::ImageUsageFlagBits::eColorAttachment |
            vk::ImageUsageFlagBits::eTransferDst,
        vk::SharingMode::eExclusive, 0, nullptr,
        vk::SurfaceTransformFlagBitsKHR::eIdentity,
        vk::CompositeAlphaFlagBitsKHR::eOpaque, vk::PresentModeKHR::eFifo,
        true);

    return device.Get().createSwapchainKHR(createInfo);
}

std::vector<vk::raii::ImageView> Swapchain::CreateSwapchainImageViews(
    vk::raii::Device& device, vk::SurfaceFormatKHR surfaceFormat)
{
    std::vector<vk::raii::ImageView> imageViews;
    std::vector<vk::Image> swapchainImages = m_Swapchain.getImages();
    imageViews.reserve(swapchainImages.size());
    for (auto image : swapchainImages)
    {
        vk::ImageSubresourceRange subresourceRange(
            vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1);
        vk::ImageViewCreateInfo createInfo(
            {}, image, vk::ImageViewType::e2D, surfaceFormat.format, {},
            subresourceRange);

        imageViews.emplace_back(device, createInfo);
    }
    return imageViews;
}