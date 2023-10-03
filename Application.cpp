#include "Application.hpp"
#include <SDL2/SDL.h>

Application::Application() : m_Running(true) {}

void Application::Run()
{
    while (m_Running)
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
        m_Video.Render();
    }
}
void Application::Update() {}