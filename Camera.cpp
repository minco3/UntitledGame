#include "Camera.hpp"
#include "glm/gtc/matrix_transform.hpp"

glm::mat4x4 Camera::GetMVP()
{
    glm::mat4x4 model = glm::mat4(1.0f);
    glm::mat4x4 view =
        glm::lookAt(position, position + lookdir, {0.0f, 0.0f, 1.0f});
    glm::mat4x4 projection =
        glm::perspective(glm::radians(45.0f), 1.0f, 0.1f, 100.0f);
    glm::mat4x4 clip = glm::mat4x4(
        1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.5f, 0.0f,
        0.0f, 0.0f, 0.5f,
        1.0f); // vulkan clip space has inverted y and half z !
    return clip * projection * view * model;
}

void Camera::look(int32_t xrel, int32_t yrel)
{
    float sensitivity = 100.0f;
    lookdir = glm::normalize(glm::vec3(
        lookdir.x * cos(xrel * sensitivity) -
            lookdir.z * sin(xrel * sensitivity),
        lookdir.y - yrel * sensitivity,
        lookdir.x * sin(xrel * sensitivity) +
            lookdir.z * cos(xrel * sensitivity)));
}