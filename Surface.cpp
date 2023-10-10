#include "Surface.hpp"
#include "Device.hpp"

Surface::Surface(Window& window, VulkanInstance& instance)
{
    if (SDL_Vulkan_CreateSurface(window(), instance(), &m_Surface) != SDL_TRUE)
    {
        LogError(
            fmt::format("Could not create SDL surface: {}", SDL_GetError()));
    }
}

VkSurfaceKHR& Surface::operator()() { return m_Surface; }

VkSurfaceCapabilitiesKHR Surface::GetSurfaceCapabilities(Device device)
{
    VkResult result;
    VkSurfaceCapabilitiesKHR surfaceCapabilities;

    result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
        device.m_PhysicalDevice, m_Surface, &surfaceCapabilities);

    LogDebug(fmt::format(
        "Surface capabiltiies:\t{}", surfaceCapabilities.currentExtent));
    return surfaceCapabilities;
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