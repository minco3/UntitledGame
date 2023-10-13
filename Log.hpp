#pragma once

#include <stdexcept>
#include <iostream>
#include <vulkan/vulkan.h>
#include <fmt/ostream.h>
#include "vulkanfmt.hpp"

void LogError(const std::string& message);

void LogWarning(const std::string& message);

void LogDebug(const std::string& message);