#pragma once

#include "Video.hpp"
#include "Camera.hpp"
#include <glm/vec2.hpp>
#include <chrono>

class Application
{
public:
    Application();
    ~Application();
    void Run();
    void Update();

private:
    Video m_Video;
    bool m_Running;
    bool m_CaptureMouse;
    Camera m_Camera;
    std::chrono::steady_clock::time_point m_LastTimePoint;
};