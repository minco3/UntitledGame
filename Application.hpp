#pragma once

#include "Video.hpp"
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
    float m_Color;
    float m_Theta;
    glm::vec2 m_RotationAxis;
};