#pragma once
#include "Instance.hpp"
#include "Log.hpp"
#include "Window.hpp"
#include <SDL2/SDL_vulkan.h>
#include <vector>
#include <vulkan/vulkan_core.h>

// ugly forward decl
class Device;

class Surface
{
public:
    Surface(Window& window, VulkanInstance& instance);
    vk::raii::SurfaceKHR& operator()();
    void GetSurfaceCapabilities(Device device);
    const std::vector<vk::SurfaceFormatKHR>
    GetCompatableSurfaceFormats(Device device);
    void GetSurfaceFormat(Device device);
    vk::SurfaceFormatKHR PickSurfaceFormat(
        const std::vector<VkSurfaceFormatKHR>& surfaceFormats,
        const VkFormat requestedFormat);

    vk::SurfaceCapabilitiesKHR surfaceCapabilities;
    vk::SurfaceFormatKHR surfaceFormat;

private:
    VkSurfaceKHR createSDLVulkanSurface(Window& window, VulkanInstance& instance);
    vk::raii::SurfaceKHR m_Surface;
};