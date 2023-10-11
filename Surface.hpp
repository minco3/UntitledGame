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
    VkSurfaceKHR& operator()();
    void GetSurfaceCapabilities(Device device);
    const std::vector<VkSurfaceFormatKHR>
    GetCompatableSurfaceFormats(Device device);
    void GetSurfaceFormat(Device device);
    VkSurfaceFormatKHR PickSurfaceFormat(
        const std::vector<VkSurfaceFormatKHR>& surfaceFormats,
        const VkFormat requestedFormat);

    VkSurfaceCapabilitiesKHR surfaceCapabilities;
    VkSurfaceFormatKHR surfaceFormat;

private:
    VkSurfaceKHR m_Surface;
};