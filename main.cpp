#include "Application.hpp"
#include "Log.hpp"
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
    catch (std::runtime_error& e)
    {
        LogError(fmt::format("Runtime Error: {}\n", e.what()));
        exit(1);
    }
    SDL_Quit();

    return 0;
}