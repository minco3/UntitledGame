#pragma once

#include "Camera.hpp"
#include "DirectoryWatcher.hpp"
#include "Video.hpp"
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
    DirectoryWatcher m_DirectoryWatcher;
    std::chrono::steady_clock::time_point m_LastTimePoint;
};