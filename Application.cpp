#include "Application.hpp"
#include <SDL2/SDL.h>

Application::Application() {}

void Application::Run()
{
    m_Running = true;
    while (m_Running)
    {
        Update();
        m_Video.Render();
    }
}
void Application::Update()
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
        case SDL_WINDOWEVENT:
            switch (event.window.event)
            {
            case SDL_WINDOWEVENT_CLOSE:
                m_Running = false;
                break;
            }
        }
    }
}