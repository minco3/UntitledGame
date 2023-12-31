#include "Log.hpp"

void LogError(const std::string& message)
{
    std::string errorMessage = fmt::format("ERROR: {}", message);
    throw std::runtime_error(errorMessage);
}

void LogWarning(const std::string& message)
{
    fmt::print(std::cerr, "WARNING: {}\n", message);
}
#define DEBUG
void LogDebug(const std::string& message)
{
#ifdef DEBUG
    fmt::print(std::cout, "DEBUG: {}\n", message);
#endif
}