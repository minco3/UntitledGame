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
        : name(_name)
    {
        m_Device.CreateShaderModule(
            m_Device, &vertShaderCreateInfo, nullptr, &vertShaderModule);
            result);
        m_Device.CreateShaderModule(
            m_Device, &fragShaderCreateInfo, nullptr, &fragShaderModule);
    }
    std::string name;
    vk::ShaderModule vertShaderModule;
    vk::ShaderModule fragShaderModule;
};

std::vector<Shader> LoadShaders(vk::raii::Device& device);