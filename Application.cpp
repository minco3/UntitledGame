#include "Application.hpp"
#include "imgui/imgui_impl_sdl2.h"
#include "imgui/imgui_impl_vulkan.h"
#include <SDL2/SDL.h>

Application::Application()
{
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    m_Video.InitImGui();
}

Application::~Application()
{
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
}

void Application::Run()
{
    m_Running = true;
    while (m_Running)
    {
        Update();
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();
        ImGui::ShowDemoWindow();
        m_Video.Render();
    }
}
void Application::Update()
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        ImGui_ImplSDL2_ProcessEvent(&event);
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
    m_Video.UpdateUnformBuffers(m_Theta);
    m_Theta += 0.1f;
}