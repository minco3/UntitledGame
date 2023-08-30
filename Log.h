#pragma once

#include <stdexcept>
#include <iostream>
#include <vulkan/vulkan.h>
#include <fmt/ostream.h>

void LogError(const std::string& message)
{
    std::string errorMessage = fmt::format("ERROR: {}", message);
    throw std::runtime_error(errorMessage);
}

void LogWarning(const std::string& message)
{
    fmt::println(std::cerr, "WARNING: {}", message);
}

void LogVulkanError(const std::string& message, VkResult result)
{
    if (result != VK_SUCCESS)
    {
        std::string errorMessage = fmt::format("ERROR: {}: {}", message, result);
        throw std::runtime_error(errorMessage);
    }
}