#include "Camera.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include <SDL2/SDL_keycode.h>
#include <algorithm>

constexpr const float sensitivity = 0.2f;

glm::mat4x4 Camera::GetMVP()
{
    float yaw_radians = glm::radians(yaw);
    float pitch_radians = glm::radians(pitch);
    lookdir = glm::vec3(
        -sin(yaw_radians) * cos(pitch_radians),
        -cos(yaw_radians) * cos(pitch_radians), sin(pitch_radians));

    glm::mat4x4 model = glm::mat4(1.0f);
    glm::mat4x4 view =
        glm::lookAt(position, position + lookdir, {0.0f, 0.0f, 1.0f});
    glm::mat4x4 projection =
        glm::perspective(glm::radians(45.0f), aspect, 0.1f, 100.0f);
    glm::mat4x4 clip = glm::mat4x4(
        1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.5f, 0.0f,
        0.0f, 0.0f, 0.5f,
        1.0f); // vulkan clip space has inverted y and half z !
    return clip * projection * view * model;
}

void Camera::move(std::chrono::nanoseconds deltaT)
{
    float seconds =
        std::chrono::duration_cast<std::chrono::duration<float>>(deltaT)
            .count();

    velocity = glm::vec3(0.0f);

    if (movementBits & Directions::eForward)
        velocity += lookdir;
    if (movementBits & Directions::eBack)
        velocity -= lookdir;
    if (movementBits & Directions::eRight)
        velocity += glm::normalize(glm::cross(lookdir, upVector));
    if (movementBits & Directions::eLeft)
        velocity -= glm::normalize(glm::cross(lookdir, upVector));
    if (movementBits & Directions::eUp)
        velocity += upVector;
    if (movementBits & Directions::eDown)
        velocity -= upVector;

    position += velocity * seconds;
}

void Camera::processEvent(SDL_Event& event)
{
    switch (event.type)
    {
    case SDL_MOUSEMOTION:
        yaw += (event.motion.xrel * sensitivity);
        pitch = std::clamp(
            pitch - (event.motion.yrel * sensitivity), -90.0f, 90.0f);
        break;
    case SDL_KEYDOWN:
    case SDL_KEYUP:
        vk::Flags<Directions> mask;
        switch (event.key.keysym.sym)
        {
        case SDLK_w:
            mask = Directions::eForward;
            break;
        case SDLK_s:
            mask = Directions::eBack;
            break;
        case SDLK_d:
            mask = Directions::eRight;
            break;
        case SDLK_a:
            mask = Directions::eLeft;
            break;
        case SDLK_SPACE:
            mask = Directions::eUp;
            break;
        case SDLK_c:
            mask = Directions::eDown;
            break;
        }
        if (event.type == SDL_KEYDOWN)
        {
            movementBits |= mask;
        }
        else if (event.type == SDL_KEYUP)
        {
            movementBits &= ~mask;
        }
        break;
    }
}

void Camera::setAspect(uint32_t width, uint32_t height)
{
    aspect = static_cast<float>(width) / static_cast<float>(height);
}