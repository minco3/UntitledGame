#include "Surface.hpp"

Surface::Surface(Window& window, VulkanInstance& instance)
{
    if (SDL_Vulkan_CreateSurface(window(), instance(), &m_Surface) !=
        SDL_TRUE)
    {
        LogError(fmt::format(
            "Could not create SDL surface: {}", SDL_GetError()));
    }
}

VkSurfaceKHR Surface::operator()()
{
    return m_Surface;
}