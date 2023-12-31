#pragma once

#include <SDL2/SDL_video.h>
#include <glm/vec2.hpp>
#include <vulkan/vulkan_core.h>
#include <vector>

class Window
{
public:
    Window(const char* title, glm::i32vec2 pos, glm::i32vec2 size, uint32_t flags);
    ~Window();
    SDL_Window* Get();
    std::vector<const char*> GetRequiredExtensionNames() const;
private:
    SDL_Window* m_Window;
};