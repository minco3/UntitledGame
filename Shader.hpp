#pragma once

#include "Device.hpp"
#include "Log.hpp"
#include <filesystem>
#include <map>
#include <optional>
#include <string>
#include <vector>
#include <vulkan/vulkan_raii.hpp>
#include <variant>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

typedef std::variant<glm::vec3, glm::mat4> ShaderType;

std::map<std::string, vk::raii::ShaderModule>
LoadShaders(vk::raii::Device& device);
std::optional<vk::raii::ShaderModule>
CompileShader(vk::raii::Device& device, const std::string& shaderName);
std::vector<uint8_t> CompileShaderFile(const std::filesystem::path& filePath);
void ReflexShader(const std::vector<uint8_t>& shaderSource);