#pragma once
#include "glm/mat3x4.hpp"

struct UniformBufferObject
{
    glm::mat3x4 rotation;
    float color;
};