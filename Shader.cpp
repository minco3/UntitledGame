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
        std::filesystem::path(WORKING_DIRECTORY)
            .append(std::string("build"))
            .append(std::string("shader"));
    for (std::filesystem::directory_entry entry :
         std::filesystem::directory_iterator(shader_directory))
    {
        // shaderc::Compiler c;
        if (entry.path().extension().string() == ".spv") // a.frag(.spv)
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
            if (shaderType.string() == ".vert")
            {
                if (!std::filesystem::exists(fragShaderPath))
                {
                    LogWarning(fmt::format(
                        "Could not find fragment shader for shader {}", name));
                    continue;
                }
            }
            else if (shaderType.string() == ".frag")
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

std::optional<Shader>
CompileShader(vk::raii::Device& device, const std::string& shaderName)
{
    std::filesystem::path shader_source_directory =
        std::filesystem::path(WORKING_DIRECTORY).append(std::string("shader"));

    // we currently only support vertex/fragment shader combos
    // TODO: change this.

    std::filesystem::path vertShaderPath =
                              std::filesystem::path(shader_source_directory)
                                  .append(shaderName + ".vert"),
                          fragShaderPath =
                              std::filesystem::path(shader_source_directory)
                                  .append(shaderName + ".frag");

    std::vector<uint32_t> vertShaderCode = CompileShaderFile(vertShaderPath);
    std::vector<uint32_t> fragShaderCode = CompileShaderFile(fragShaderPath);

    if (!vertShaderCode.size() || !fragShaderCode.size())
    {
        return {};
    }

    vk::ShaderModuleCreateInfo vertShaderCreateInfo(
        {}, static_cast<uint32_t>(vertShaderCode.size() * sizeof(uint32_t)),
        vertShaderCode.data());

    vk::ShaderModuleCreateInfo fragShaderCreateInfo(
        {}, static_cast<uint32_t>(fragShaderCode.size() * sizeof(uint32_t)),
        fragShaderCode.data());

    return std::optional<Shader>{
        {device, shaderName, vertShaderCreateInfo, fragShaderCreateInfo}};
}

std::vector<uint32_t> CompileShaderFile(const std::filesystem::path& filePath)
{

    if (!std::filesystem::exists(filePath))
    {
        LogWarning(
            fmt::format("Shader source file: {} missing!", filePath.string()));
        return {};
    }

    shaderc::Compiler c;

    std::ifstream vertSourceFile(filePath, std::ios::binary);
    std::string shaderSource{
        std::istreambuf_iterator<char>(vertSourceFile),
        std::istreambuf_iterator<char>()};

    shaderc_shader_kind shaderKind;

    if (filePath.extension().string() == ".vert")
    {
        shaderKind = shaderc_shader_kind::shaderc_vertex_shader;
    }
    else if (filePath.extension().string() == ".frag")
    {
        shaderKind = shaderc_shader_kind::shaderc_fragment_shader;
    }
    else
    {
        LogWarning(fmt::format(
            "Unknown Shader Type: {}", filePath.extension().string()));
    }

    auto result = c.CompileGlslToSpv(
        shaderSource, shaderKind, filePath.filename().string().data(),
        shaderc::CompileOptions());

    shaderc_compilation_status status = result.GetCompilationStatus();
    if (status !=
        shaderc_compilation_status::shaderc_compilation_status_success)
    {
        LogWarning(fmt::format(
            "Shader compilation error: {}", result.GetErrorMessage()));
        return {};
    }

    return std::vector<uint32_t>(result.cbegin(), result.cend());
}
