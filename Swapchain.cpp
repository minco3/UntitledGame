#include "Swapchain.hpp"

Swapchain::Swapchain(Device& device, Surface& surface)
{

    CreateSwapchain(device, surface);
    CreateSwapchainImageViews(device, surface);
}

void Swapchain::CreateSwapchain(Device& device, Surface& surface)
{
    surface.GetSurfaceCapabilities(device);
    surface.GetSurfaceFormat(device);

    VkResult result;
    VkSwapchainCreateInfoKHR swapchainCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = surface(),
        .minImageCount = surface.surfaceCapabilities.minImageCount + 1,
        .imageFormat = surface.surfaceFormat.format,
        .imageColorSpace = surface.surfaceFormat.colorSpace,
        .imageExtent = surface.surfaceCapabilities.currentExtent,
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
                      VK_IMAGE_USAGE_TRANSFER_DST_BIT,
        .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = VK_PRESENT_MODE_FIFO_KHR,
        .clipped = true};

    result = vkCreateSwapchainKHR(
        device(), &swapchainCreateInfo, nullptr, &m_Swapchain);
    LogVulkanError("Failed to create swapchain", result);
}

void Swapchain::CreateSwapchainImageViews(Device& device, Surface& surface)
{
    VkResult result;
    std::vector<VkImage> swapchainImages = GetSwapchainImages(device);
    m_SwapchainImageViews.resize(swapchainImages.size());
    for (size_t i = 0; i < swapchainImages.size(); i++)
    {
        VkImageView& swapchainImageView = m_SwapchainImageViews.at(i);
        VkImageViewCreateInfo swapchainImageViewCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image = swapchainImages.at(i),
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = surface.surfaceFormat.format,
            .components =
                {.r = VK_COMPONENT_SWIZZLE_IDENTITY,
                 .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                 .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                 .a = VK_COMPONENT_SWIZZLE_IDENTITY},
            .subresourceRange = {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1}};
        result = vkCreateImageView(
            device(), &swapchainImageViewCreateInfo, nullptr,
            &swapchainImageView);
        LogVulkanError(fmt::format("Failed to create swapchain image view {}", i), result);
    }
}

std::vector<VkImage> Swapchain::GetSwapchainImages(Device& device)
{
    VkResult result;
    uint32_t swapchainImageCount = 0;
    result = vkGetSwapchainImagesKHR(
        device(), m_Swapchain, &swapchainImageCount, nullptr);
    LogVulkanError("Failed to get swapchain image count", result);
    std::vector<VkImage> swapchainImages(
        static_cast<size_t>(swapchainImageCount));
    result = vkGetSwapchainImagesKHR(
        device(), m_Swapchain, &swapchainImageCount, swapchainImages.data());
    LogVulkanError("Failed to get swapchain images", result);
    return swapchainImages;
}

VkSwapchainKHR& Swapchain::operator()()
{
    return m_Swapchain;
}
