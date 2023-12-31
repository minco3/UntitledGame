#pragma once
#include "Instance.hpp"
#include "Log.hpp"
#include "Window.hpp"
#include <SDL2/SDL_vulkan.h>
#include <vector>
#include <vulkan/vulkan_raii.hpp>

// ugly forward decl
class Device;

class Surface
{
public:
    Surface(Window& window, VulkanInstance& instance);
    void GetSurfaceCapabilities(Device& device);
    const std::vector<vk::SurfaceFormatKHR>
    GetCompatableSurfaceFormats(Device& device);
    void GetSurfaceFormat(Device& device);
    vk::SurfaceFormatKHR PickSurfaceFormat(
        const std::vector<vk::SurfaceFormatKHR>& surfaceFormats,
        const vk::Format requestedFormat);

    vk::raii::SurfaceKHR& Get();

    vk::SurfaceCapabilitiesKHR surfaceCapabilities;
    vk::SurfaceFormatKHR surfaceFormat;

private:
    VkSurfaceKHR createSDLVulkanSurface(SDL_Window* window, vk::raii::Instance& instance);
    vk::raii::SurfaceKHR m_Surface;
};