#pragma once

#include <glm/common.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

struct Camera
{
    glm::mat4x4 GetMVP();
    void look(int32_t xrel, int32_t yrel);
    glm::vec3 position, lookdir = {1.0f, 0.0f, 0.0f};
};