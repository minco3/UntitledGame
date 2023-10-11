#include "Surface.hpp"
#include "Device.hpp"
#include <algorithm>

Surface::Surface(Window& window, VulkanInstance& instance)
{
    if (SDL_Vulkan_CreateSurface(window(), instance(), &m_Surface) != SDL_TRUE)
    {
        LogError(
            fmt::format("Could not create SDL surface: {}", SDL_GetError()));
    }
}

VkSurfaceKHR Surface::operator()() { return m_Surface; }

void Surface::GetSurfaceCapabilities(Device device)
{
    VkResult result;

    result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
        device.m_PhysicalDevice, m_Surface, &surfaceCapabilities);

    LogDebug(fmt::format(
        "Surface capabiltiies:\t{}", surfaceCapabilities.currentExtent));
}

const std::vector<VkSurfaceFormatKHR>
Surface::GetCompatableSurfaceFormats(Device device)
{
    VkResult result;
    uint32_t surfaceFormatCount = 0;
    result = vkGetPhysicalDeviceSurfaceFormatsKHR(
        device.m_PhysicalDevice, m_Surface, &surfaceFormatCount, nullptr);
    LogVulkanError("Failed to get supported surface format count", result);
    std::vector<VkSurfaceFormatKHR> surfaceFormats(
        static_cast<size_t>(surfaceFormatCount));
    result = vkGetPhysicalDeviceSurfaceFormatsKHR(
        device.m_PhysicalDevice, m_Surface, &surfaceFormatCount,
        surfaceFormats.data());
    LogVulkanError(
        "Failed to get surface formats supported by the device", result);

    LogDebug("Supported Formats:");
    for (const auto& surfaceFormat : surfaceFormats)
    {
        LogDebug(fmt::format(
            "\t{}, {}", surfaceFormat.colorSpace, surfaceFormat.format));
    }
    return surfaceFormats;
}

VkSurfaceFormatKHR Surface::PickSurfaceFormat(
    const std::vector<VkSurfaceFormatKHR>& surfaceFormats,
    const VkFormat requestedFormat)
{
    VkSurfaceFormatKHR surfaceFormat = {
        .format = VK_FORMAT_UNDEFINED,
        .colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};
    if (surfaceFormats.size() == 1 &&
        surfaceFormats.at(0).format == VK_FORMAT_UNDEFINED)
    {
        surfaceFormat = {
            .format = VK_FORMAT_R8G8B8A8_UNORM,
            .colorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR};
    }
    else if (
        std::find_if(
            surfaceFormats.begin(), surfaceFormats.end(),
            [requestedFormat](VkSurfaceFormatKHR surfaceFormat) {
                return surfaceFormat.format == requestedFormat;
            }) != surfaceFormats.end())
    {
        surfaceFormat.format = requestedFormat;
    }

    // fallback
    if (surfaceFormat.format == VK_FORMAT_UNDEFINED)
    {
        surfaceFormat = surfaceFormats.at(0);
        LogWarning(fmt::format(
            "Requested format not found! Falling back to: {} {}",
            surfaceFormat.format, surfaceFormat.colorSpace));
    }
    return surfaceFormat;
}

void Surface::GetSurfaceFormat(Device device)
{
    surfaceFormat = PickSurfaceFormat(
        GetCompatableSurfaceFormats(device), VK_FORMAT_B8G8R8A8_UNORM);
}