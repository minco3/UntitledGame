#include "Shader.hpp"
#include <algorithm>
#include <filesystem>
#include <shaderc/shaderc.hpp>

Shader::Shader(
    vk::raii::Device& device, const std::string& _name,
    const vk::ShaderModuleCreateInfo& vertShaderCreateInfo,
    const vk::ShaderModuleCreateInfo& fragShaderCreateInfo)
    : name(_name), vertShaderModule(device, vertShaderCreateInfo),
        fragShaderModule(device, fragShaderCreateInfo)
{
}

std::vector<Shader> LoadShaders(vk::raii::Device& device)
{
    std::vector<Shader> shaders;
    std::filesystem::path shader_directory =
        std::filesystem::path(WORKING_DIRECTORY).append("build").append("shader");
    for (std::filesystem::directory_entry entry :
         std::filesystem::directory_iterator(shader_directory))
    {
        // shaderc::Compiler c;
        if (entry.path().extension() == ".spv") // a.frag(.spv)
        {
            std::string name =
                entry.path().stem().stem().string(); // (a).frag.spv
            // check if shader already exists
            if (std::find_if(
                    shaders.begin(), shaders.end(),
                    [name](const Shader& shader)
                    { return name == shader.name; }) != shaders.end())
            {
                continue;
            }

            // if it doesnt look for the missing half
            std::filesystem::path shaderType =
                entry.path().stem().extension(); // a(.frag).spv
            std::filesystem::path vertShaderPath = shader_directory;
            vertShaderPath.append(name + ".vert.spv");
            std::filesystem::path fragShaderPath = shader_directory;
            fragShaderPath.append(name + ".frag.spv");
            if (shaderType == ".vert")
            {
                if (!std::filesystem::exists(fragShaderPath))
                {
                    LogWarning(fmt::format(
                        "Could not find fragment shader for shader {}", name));
                    continue;
                }
            }
            else if (shaderType == ".frag")
            {
                if (!std::filesystem::exists(vertShaderPath))
                {
                    LogWarning(fmt::format(
                        "Could not find vertex shader for shader {}", name));
                    continue;
                }
            }
            else
            {
                continue;
            }

            std::ifstream vertShaderCodeFile(vertShaderPath, std::ios::binary);
            if (!vertShaderCodeFile.is_open())
            {
                LogWarning(fmt::format(
                    "Requested vertex shader {} missing!",
                    vertShaderPath.string()));
            }

            std::ifstream fragShaderCodeFile(fragShaderPath, std::ios::binary);
            if (!fragShaderCodeFile.is_open())
            {
                LogWarning(fmt::format(
                    "Requested fragment shader {} missing!",
                    fragShaderPath.string()));
            }

            const std::string vertShaderCode(
                (std::istreambuf_iterator<char>(vertShaderCodeFile)),
                std::istreambuf_iterator<char>());

            std::string fragShaderCode(
                (std::istreambuf_iterator<char>(fragShaderCodeFile)),
                std::istreambuf_iterator<char>());

            vk::ShaderModuleCreateInfo vertShaderCreateInfo(
                {}, static_cast<uint32_t>(vertShaderCode.size()),
                reinterpret_cast<const uint32_t*>(vertShaderCode.data()));

            vk::ShaderModuleCreateInfo fragShaderCreateInfo(
                {}, static_cast<uint32_t>(fragShaderCode.size()),
                reinterpret_cast<const uint32_t*>(fragShaderCode.data()));

            shaders.emplace_back(
                device, name, vertShaderCreateInfo, fragShaderCreateInfo);
        }
    }
    if (shaders.size() == 0)
    {
        LogWarning("No shaders constructed!");
    }
    return shaders;
}

Shader CompileShader(vk::raii::Device& device)
{
    
}