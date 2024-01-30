#pragma once

#include "vulkanfmt.hpp"
#include <fmt/ostream.h>
#include <iostream>
#include <stdexcept>

void LogError(const std::string& message);

void LogWarning(const std::string& message);

void LogDebug(const std::string& message);