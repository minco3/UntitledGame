#include "Shader.hpp"
#include <algorithm>
#include <filesystem>

std::vector<Shader> LoadShaders(VkDevice device)
{
    std::vector<Shader> shaders;
    std::filesystem::path current_directory =
        std::filesystem::path(WORKING_DIRECTORY).append("build").append("shader");
    for (std::filesystem::directory_entry entry :
         std::filesystem::directory_iterator(current_directory))
    {
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
            std::filesystem::path vertShaderPath = current_directory;
            vertShaderPath.append(name + ".vert.spv");
            std::filesystem::path fragShaderPath = current_directory;
            fragShaderPath.append(name + ".frag.spv");
            if (shaderType == ".vert")
            {
                if (!std::filesystem::exists(
                        fragShaderPath))
                {
                    LogWarning(fmt::format(
                        "Could not find fragment shader for shader {}", name));
                    continue;
                }
            }
            else if (shaderType == ".frag")
            {
                if (!std::filesystem::exists(
                        vertShaderPath))
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

            std::ifstream vertShaderCodeFile(
                vertShaderPath, std::ios::binary);
            if (!vertShaderCodeFile.is_open())
            {
                LogWarning(fmt::format(
                    "Requested vertex shader {} missing!",
                    vertShaderPath.string()));
            }

            std::ifstream fragShaderCodeFile(
                fragShaderPath, std::ios::binary);
            if (!fragShaderCodeFile.is_open())
            {
                LogWarning(fmt::format(
                    "Requested fragment shader {} missing!",
                    fragShaderPath.string()));
            }

            std::string vertShaderCode(
                (std::istreambuf_iterator<char>(vertShaderCodeFile)),
                std::istreambuf_iterator<char>());

            std::string fragShaderCode(
                (std::istreambuf_iterator<char>(fragShaderCodeFile)),
                std::istreambuf_iterator<char>());

            VkShaderModuleCreateInfo vertShaderCreateInfo = {
                .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
                .codeSize = vertShaderCode.size(),
                .pCode =
                    reinterpret_cast<const uint32_t*>(vertShaderCode.data())};

            VkShaderModuleCreateInfo fragShaderCreateInfo = {
                .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
                .codeSize = fragShaderCode.size(),
                .pCode =
                    reinterpret_cast<const uint32_t*>(fragShaderCode.data())};

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