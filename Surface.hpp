#pragma once
#include "Log.hpp"
#include "Instance.hpp"
#include "Window.hpp"
#include <SDL2/SDL_vulkan.h>
#include <vulkan/vulkan_core.h>

class Surface
{
    public:
    Surface(Window& window, VulkanInstance& instance);
    VkSurfaceKHR operator()();
    private:
    VkSurfaceKHR m_Surface;
};