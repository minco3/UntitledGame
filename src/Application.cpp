#include "Application.hpp"
#include <SDL2/SDL.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_vulkan.h>

Application::Application() : m_LastTimePoint(std::chrono::steady_clock::now())
{
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    m_Video.InitImGui();
    vk::Extent2D extent = m_Video.GetScreenSize();
    m_Camera.setAspect(extent.width, extent.height);

    m_DirectoryWatcher.SubscribeToDirectory(
        std::filesystem::path(WORKING_DIRECTORY).append("shader"),
        [&](const std::filesystem::path& path, fileStatus fs)
        {
            if (fs == fileStatus::eModified)
            {
                m_Video.RecreatePipeline(
                    path.filename().string(),
                    std::filesystem::last_write_time(path));
            }
        });
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
        ImGui::InputFloat("yaw", &m_Camera.yaw);
        ImGui::InputFloat("pitch", &m_Camera.pitch);
        ImGui::Text(
            "Application average %.3f ms/frame (%.1f FPS)",
            1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        Update();
        ImGui::End();

        m_Video.CheckForUpdates();
        m_Video.Render();
    }
}
void Application::Update()
{
    auto deltaT = std::chrono::steady_clock::now() - m_LastTimePoint;
    m_LastTimePoint = std::chrono::steady_clock::now();
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        ImGui_ImplSDL2_ProcessEvent(&event);
        m_Camera.processEvent(event);
        switch (event.type)
        {
        case SDL_WINDOWEVENT:
            switch (event.window.event)
            {
            case SDL_WINDOWEVENT_CLOSE:
                m_Running = false;
                break;
            case SDL_WINDOWEVENT_RESIZED:
                m_Video.Resize();
                m_Camera.setAspect(event.window.data1, event.window.data2);
                break;
            }
            break;
        case SDL_MOUSEBUTTONDOWN:
            if (event.button.button == SDL_BUTTON_LEFT)
            {
                if (ImGui::IsAnyItemHovered() ||
                    ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow))
                    return;
                SDL_SetRelativeMouseMode(SDL_TRUE);
            }
            break;
        case SDL_KEYDOWN:
            switch (event.key.keysym.sym)
            {
            case SDLK_ESCAPE:
                SDL_SetRelativeMouseMode(SDL_FALSE);
                break;
            }
            break;
        }
    }
    m_Camera.move(deltaT);
    m_Camera.UpdateMVP();
    m_Video.UpdateUniformBuffers(m_Camera.MVP);
}