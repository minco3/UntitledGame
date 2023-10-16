#pragma once

#include "Log.hpp"
#include <string>
#include <vector>
#include <vulkan/vulkan_raii.hpp>

struct Shader
{
    Shader(
        vk::raii::Device& device, const std::string& _name,
        const vk::ShaderModuleCreateInfo& vertShaderCreateInfo,
        const vk::ShaderModuleCreateInfo& fragShaderCreateInfo)
        : name(_name), vertShaderModule(device, vertShaderCreateInfo),
          fragShaderModule(device, fragShaderCreateInfo)
    {
    }
    std::string name;
    vk::raii::ShaderModule vertShaderModule;
    vk::raii::ShaderModule fragShaderModule;
};

std::vector<Shader> LoadShaders(vk::raii::Device& device);