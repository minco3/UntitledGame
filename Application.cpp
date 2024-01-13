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
}

void Application::Run()
{
    m_Running = true;
    while (m_Running)
    {
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();
        // ImGui::ShowDemoWindow();
        ImGui::Begin("Hello, world!");
        ImGui::SliderFloat("color", &m_Color, 0.0f, 360.0f);
        ImGui::SliderFloat("theta", &m_Theta, 0.0f, 360.0f);
        ImGui::SliderFloat("x", &m_RotationAxis.x, -1.0f, 1.0f);
        ImGui::SliderFloat("y", &m_RotationAxis.y, -1.0f, 1.0f);
        Update();
        ImGui::End();

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
    m_Video.UpdateUnformBuffers(m_Color, m_Theta, m_RotationAxis);
}