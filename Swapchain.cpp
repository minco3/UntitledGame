#include "Swapchain.hpp"

Swapchain::Swapchain(Device& device, Surface& surface)
    : m_Swapchain(CreateSwapchain(device, surface)),
      m_ImageCount(m_SwapchainImageViews.size())
{
    CreateSwapchainImageViews(device.Get(), surface.surfaceFormat);
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

void Swapchain::CreateSwapchainImageViews(
    vk::raii::Device& device, vk::SurfaceFormatKHR surfaceFormat)
{
    VkResult result;
    std::vector<vk::Image> swapchainImages = m_Swapchain.getImages();
    m_SwapchainImageViews.reserve(swapchainImages.size());
    for (size_t i = 0; i < swapchainImages.size(); i++)
    {

        vk::ImageSubresourceRange subresourceRange(
            vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1);
        vk::ImageViewCreateInfo createInfo(
            {}, swapchainImages.at(i), vk::ImageViewType::e2D,
            surfaceFormat.format, {}, subresourceRange);

        m_SwapchainImageViews.emplace_back(device, createInfo);
    }
}