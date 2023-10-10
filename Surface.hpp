#pragma once
#include "Instance.hpp"
#include "Log.hpp"
#include "Window.hpp"
#include <SDL2/SDL_vulkan.h>
#include <vector>
#include <vulkan/vulkan_core.h>

class Device;

class Surface
{
public:
    Surface(Window& window, VulkanInstance& instance);
    VkSurfaceKHR& operator()();
    VkSurfaceCapabilitiesKHR GetSurfaceCapabilities(Device device);
    const std::vector<VkSurfaceFormatKHR>
    GetCompatableSurfaceFormats(Device device);

private:
    VkSurfaceKHR m_Surface;
};