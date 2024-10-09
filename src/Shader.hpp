#pragma once

#include "Device.hpp"
#include "Log.hpp"
#include <filesystem>
#include <map>
#include <optional>
#include <string>
#include <variant>
#include <vector>
#include <vulkan/vulkan_raii.hpp>

struct Shader
{
    Shader(
        vk::raii::Device& device, const std::string& _name,
        const vk::ShaderModuleCreateInfo& vertShaderCreateInfo,
        const vk::ShaderModuleCreateInfo& fragShaderCreateInfo);
    std::string name;
    vk::raii::ShaderModule vertShaderModule;
    vk::raii::ShaderModule fragShaderModule;
};

std::map<std::string, vk::raii::ShaderModule>
LoadShaders(vk::raii::Device& device);
std::optional<vk::raii::ShaderModule>
CompileShader(vk::raii::Device& device, const std::string& shaderName);
std::vector<uint32_t> CompileShaderFile(const std::filesystem::path& filePath);
void ReflexShader(const std::span<const uint32_t>& shaderSource);
