#define GLM_SWIZZLE
#include "Application.hpp"
#include "imgui/imgui_impl_sdl2.h"
#include "imgui/imgui_impl_vulkan.h"
#include <SDL2/SDL.h>
#include <glm/gtx/perpendicular.hpp>

Application::Application()
{
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    m_Video.InitImGui();
}

Application::~Application() {}

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
        ImGui::InputFloat3("position", &m_Camera.position.x);
        ImGui::InputFloat3("lookdir", &m_Camera.lookdir.x);
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
            break;
        case SDL_KEYDOWN:
            switch (event.key.keysym.sym)
            {
            case SDLK_w:
                m_Camera.position += m_Camera.lookdir;
                break;
            case SDLK_a:
                m_Camera.position += glm::normalize(glm::perp(
                    glm::vec3(-m_Camera.lookdir.y, m_Camera.lookdir.x, 0.0f),
                    m_Camera.lookdir));
                break;
            case SDLK_s:
                m_Camera.position -= m_Camera.lookdir;
                break;
            case SDLK_d:
                m_Camera.position -= glm::normalize(glm::perp(
                    glm::vec3(-m_Camera.lookdir.y, m_Camera.lookdir.x, 0.0f),
                    m_Camera.lookdir));
                break;
            case SDLK_SPACE:
                m_Camera.position += glm::vec3(0.0f, 0.0f, 1.0f);
                break;
            case SDLK_c:
                m_Camera.position -= glm::vec3(0.0f, 0.0f, 1.0f);
                break;
            case SDLK_LEFT:
                m_Camera.lookdir = glm::normalize(
                    m_Camera.lookdir +
                    0.2f * glm::normalize(glm::vec3(
                               -m_Camera.lookdir.y, m_Camera.lookdir.x, 0.0f)));
                break;
            case SDLK_RIGHT:
                m_Camera.lookdir = glm::normalize(
                    m_Camera.lookdir -
                    0.2f * glm::normalize(glm::vec3(
                               -m_Camera.lookdir.y, m_Camera.lookdir.x, 0.0f)));
                break;
            case SDLK_UP:
                m_Camera.lookdir = glm::normalize(
                    m_Camera.lookdir +
                    0.2f * glm::normalize(glm::vec3(
                               -m_Camera.lookdir.z, 0.0f, glm::distance(m_Camera.lookdir.x, m_Camera.lookdir.y))));
                break;
            case SDLK_DOWN:
                m_Camera.lookdir = glm::normalize(
                    m_Camera.lookdir -
                    0.2f * glm::normalize(glm::vec3(
                               -m_Camera.lookdir.z, 0.0f, glm::distance(m_Camera.lookdir.x, m_Camera.lookdir.y))));
                break;
            }
            break;
        }
    }
    m_Video.UpdateUnformBuffers(m_Camera.GetMVP());
}