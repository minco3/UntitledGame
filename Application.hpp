#pragma once

#include "Video.hpp"
#include "Camera.hpp"
#include <glm/vec2.hpp>

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
    Camera m_Camera;
};