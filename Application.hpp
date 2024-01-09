#pragma once

#include "Video.hpp"

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
    float m_Theta;
};