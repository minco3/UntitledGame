#include "Surface.hpp"
#include "Device.hpp"
#include <algorithm>

Surface::Surface(SDL_Window* window, vk::raii::Instance& instance)
    : m_Surface(instance, createSDLVulkanSurface(window, instance))
{
}

VkSurfaceKHR createSDLVulkanSurface(SDL_Window* window, vk::raii::Instance& instance)
{
    VkSurfaceKHR surface;
    if (!window)
    {
        LogError("Window pointer not initialized correctly");
    }
    if (SDL_Vulkan_CreateSurface(
            window, static_cast<VkInstance>(*instance), &surface) !=
        SDL_TRUE)
    {
        LogError(
            fmt::format("Could not create SDL surface: {}", SDL_GetError()));
    }
    return surface;
}

void Surface::GetSurfaceCapabilities(Device& device)
{
    surfaceCapabilities = device.GetSurfaceCapabilities(m_Surface);

    LogDebug(fmt::format(
        "Surface capabiltiies:\t{}", surfaceCapabilities.currentExtent));
}

const std::vector<vk::SurfaceFormatKHR>
Surface::GetCompatableSurfaceFormats(Device& device)
{
    return device.GetCompatableSurfaceFormats(m_Surface);
}

vk::SurfaceFormatKHR Surface::PickSurfaceFormat(
    const std::vector<vk::SurfaceFormatKHR>& surfaceFormats,
    const vk::Format requestedFormat)
{
    vk::SurfaceFormatKHR surfaceFormat(vk::Format::eUndefined, vk::ColorSpaceKHR::eSrgbNonlinear);
    if (surfaceFormats.size() == 1 &&
        surfaceFormats.front().format == vk::Format::eUndefined)
    {
        surfaceFormat = {vk::Format::eR8G8B8A8Unorm, vk::ColorSpaceKHR::eSrgbNonlinear};
    }
    else if (
        std::find_if(
            surfaceFormats.begin(), surfaceFormats.end(),
            [requestedFormat](vk::SurfaceFormatKHR surfaceFormat) {
                return surfaceFormat.format == requestedFormat;
            }) != surfaceFormats.end())
    {
        surfaceFormat.format = requestedFormat;
    }

    // fallback
    if (surfaceFormat.format == vk::Format::eUndefined)
    {
        surfaceFormat = surfaceFormats.front();
        LogWarning(fmt::format(
            "Requested format not found! Falling back to: {} {}",
            surfaceFormat.format, surfaceFormat.colorSpace));
    }
    return surfaceFormat;
}

vk::raii::SurfaceKHR& Surface::Get()
{
    return m_Surface;
}

void Surface::GetSurfaceFormat(Device& device)
{
    surfaceFormat = PickSurfaceFormat(
        GetCompatableSurfaceFormats(device), vk::Format::eB8G8R8A8Unorm);
}