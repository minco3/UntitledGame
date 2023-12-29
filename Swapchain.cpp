#include "Swapchain.hpp"

Swapchain::Swapchain(
    vk::raii::Device& device, vk::raii::SurfaceKHR& surface,
    vk::SurfaceCapabilitiesKHR& surfaceCapabilities,
    vk::SurfaceFormatKHR& surfaceFormat)
    : m_Swapchain(
          device,
          GetCreateInfo(device, surface, surfaceCapabilities, surfaceFormat))
{
    CreateSwapchainImageViews(device, surfaceFormat);
}

vk::SwapchainCreateInfoKHR Swapchain::GetCreateInfo(
    vk::raii::Device& device, vk::raii::SurfaceKHR& surface,
    vk::SurfaceCapabilitiesKHR& surfaceCapabilities,
    vk::SurfaceFormatKHR& surfaceFormat)
{

    return {
        {},
        *surface,
        surfaceCapabilities.minImageCount + 1,
        surfaceFormat.format,
        surfaceFormat.colorSpace,
        surfaceCapabilities.currentExtent,
        1,
        vk::ImageUsageFlagBits::eColorAttachment |
            vk::ImageUsageFlagBits::eTransferDst,
        vk::SharingMode::eExclusive,
        0,
        nullptr,
        vk::SurfaceTransformFlagBitsKHR::eIdentity,
        vk::CompositeAlphaFlagBitsKHR::eOpaque,
        vk::PresentModeKHR::eFifo,
        true};
}

void Swapchain::CreateSwapchainImageViews(
    vk::raii::Device& device, vk::SurfaceFormatKHR surfaceFormat)
{
    VkResult result;
    std::vector<vk::Image> swapchainImages = m_Swapchain.getImages();
    m_SwapchainImageViews.resize(swapchainImages.size());
    for (size_t i = 0; i < swapchainImages.size(); i++)
    {

        vk::ImageSubresourceRange subresourceRange(
            vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1);
        vk::ImageViewCreateInfo createInfo(
            {}, swapchainImages.at(i),
            vk::ImageViewType::e2D, surfaceFormat.format, {},
            subresourceRange);

        m_SwapchainImageViews.at(i) = vk::raii::ImageView(device, createInfo);
    }
}