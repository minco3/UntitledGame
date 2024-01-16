#pragma once

#include "Device.hpp"
#include "Log.hpp"
#include <map>
#include <optional>
#include <string>
#include <vector>
#include <vulkan/vulkan_raii.hpp>

std::map<std::string, vk::raii::ShaderModule>
LoadShaders(vk::raii::Device& device);
std::optional<vk::raii::ShaderModule>
CompileShader(vk::raii::Device& device, const std::string& shaderName);
std::vector<uint32_t> CompileShaderFile(const std::filesystem::path& filePath);