#include "Application.hpp"
#include <stdexcept>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_vulkan.h>
#include <array>
#include <filesystem>
#include <fmt/format.h>
#include <fmt/ostream.h>
#include <fstream>
#include <glm/glm.hpp>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <vector>
#include <vulkan/vk_enum_string_helper.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_beta.h>

Application::Application() :m_Running(true){}

void Application::Run() {}
void Application::Update()
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
}