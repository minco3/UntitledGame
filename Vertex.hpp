#pragma once

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <vector>

struct Vertex
{
    glm::vec3 pos;
    glm::vec3 color;
};

const std::vector<Vertex> vertices = {
    {{0.5f,  0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}},
    {{0.0f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}},
    {{-0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}}};
