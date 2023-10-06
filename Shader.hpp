#pragma once

#include <string>
#include <vulkan/vulkan_core.h>
#include "Log.hpp"

struct Shader
{
    Shader(
        VkDevice device, const std::string& _name,
        const VkShaderModuleCreateInfo& vertShaderCreateInfo,
        const VkShaderModuleCreateInfo& fragShaderCreateInfo)
        : m_Device(device), name(_name)
    {
        VkResult result;
        result = vkCreateShaderModule(
            m_Device, &vertShaderCreateInfo, nullptr, &vertShaderModule);
        LogVulkanError(
            fmt::format("failed to create vertex shader module for {}", name),
            result);
        result = vkCreateShaderModule(
            m_Device, &fragShaderCreateInfo, nullptr, &fragShaderModule);
        LogVulkanError(
            fmt::format("failed to create fragment shader module for {}", name),
            result);
    }
    void DestroyShaderModules()
    {
        vkDestroyShaderModule(m_Device, fragShaderModule, nullptr);
        vkDestroyShaderModule(m_Device, vertShaderModule, nullptr);
    }
    VkDevice m_Device;
    std::string name;
    VkShaderModule vertShaderModule;
    VkShaderModule fragShaderModule;
};