#include <string.h>
#include <vulkan/vulkan_core.h>

enum class ShaderType
{
    Vertex,
    Fragment
};

struct Shader
{
    ShaderType type;
    std::string filePath;
    VkShaderModule shaderModule;
};