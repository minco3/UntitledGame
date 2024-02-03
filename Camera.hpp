#pragma once

#include <SDL2/SDL_events.h>
#include <chrono>
#include <glm/common.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <vulkan/vulkan.hpp>

constexpr const glm::vec3 upVector{0.0f, 0.0f, 1.0f};

enum class Directions
{
    eForward = 0x00000001,
    eBack = 0x00000002,
    eRight = 0x00000004,
    eLeft = 0x00000008,
    eUp = 0x00000010,
    eDown = 0x00000020,
    eSprint = 0x00000040
};

template <> struct vk::FlagTraits<Directions>
{
    static constexpr bool isBitmask = true;
    static constexpr vk::Flags<Directions> allFlags =
        Directions::eForward | Directions::eBack | Directions::eRight |
        Directions::eLeft | Directions::eUp | Directions::eDown |
        Directions::eSprint;
};

struct Camera
{
    void UpdateMVP();
    void move(std::chrono::nanoseconds deltaT);
    void processEvent(SDL_Event& event);
    void setAspect(uint32_t width, uint32_t height);
    glm::vec3 position = {0.0f, 3.0f, 0.0f}, lookdir, velocity;
    glm::mat4 MVP{};
    float yaw{}, pitch{};
    float aspect = 1.0f;
    vk::Flags<Directions> movementBits;
};