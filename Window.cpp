#include "Window.hpp"
#include "Log.hpp"
#include <glm/vec2.hpp>
#include <sdl2/SDL_vulkan.h>

Window::Window(
    const char* title, glm::i32vec2 pos, glm::i32vec2 size, uint32_t flags)
{
    m_Window = SDL_CreateWindow(title, pos.x, pos.y, size.x, size.y, flags);
    if (m_Window == nullptr)
    {
        LogError(fmt::format("Error creating SDL window: {}", SDL_GetError()));
    }
}

Window::~Window() {
    SDL_DestroyWindow(m_Window);
}

std::vector<const char *> Window::GetRequiredExtensionNames() const
{
    unsigned int numExtentions;
    if (SDL_Vulkan_GetInstanceExtensions(m_Window, &numExtentions, nullptr) !=
        SDL_TRUE)
    {
        LogError("Error getting SDL required vulkan exension count");
    }
    std::vector<const char*> sdlExtNames(numExtentions);
    if (SDL_Vulkan_GetInstanceExtensions(
            m_Window, &numExtentions, sdlExtNames.data()) != SDL_TRUE)
    {
        LogError("Error getting SDL required vulkan extensions");
    }
    return sdlExtNames;
}

VkSurfaceKHR Window::CreateSDLSurface(VkInstance instance)
{
    VkSurfaceKHR surface;
    if (SDL_Vulkan_CreateSurface(m_Window, instance, &surface) != SDL_TRUE)
    {
        LogError(fmt::format("Could not create SDL surface: {}", SDL_GetError()));
    }
    return surface;
}