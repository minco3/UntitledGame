#include "Application.hpp"
#include "Log.hpp"
#include "vulkan/vulkan.hpp"
#include <SDL2/SDL.h>

// #define DEBUG

int main(int argc, char** argv)
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0)
    {
        LogError(fmt::format("Error initializing sdl: {}", SDL_GetError()));
    }
    try
    {
        Application app;
        app.Run();
    }
    catch (vk::SystemError& e) // taken from VulkanSamples
    {
        LogError(fmt::format("Vulkan Error: {}\n", e.what()));
        exit(-1);
    }
    catch (std::exception& e)
    {
        LogError(fmt::format("std::exception: {}\n", e.what()));
        exit(-1);
    }
    catch (...)
    {
        LogError(fmt::format("Unknown Error"));
        exit(-1);
    }
    SDL_Quit();

    return 0;
}