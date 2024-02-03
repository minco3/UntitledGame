#pragma once
#include "glm/mat4x4.hpp"
#include "glm/vec3.hpp"

struct UniformBufferObject
{
    glm::mat4 MVP;
    glm::vec3 color;
};