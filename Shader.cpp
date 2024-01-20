#include "Shader.hpp"
#include "spirv_reflect.h"
#include <algorithm>
#include <shaderc/shaderc.hpp>

std::map<std::string, vk::raii::ShaderModule>
LoadShaders(vk::raii::Device& device)
{
    std::map<std::string, vk::raii::ShaderModule> shaders;
    std::filesystem::path shader_directory =
        std::filesystem::path(WORKING_DIRECTORY)
            .append(std::string("build"))
            .append(std::string("shader"));
    for (std::filesystem::directory_entry entry :
         std::filesystem::directory_iterator(shader_directory))
    {
        if (entry.path().extension().string() == ".spv") // a.frag(.spv)
        {
            std::ifstream shaderFile(entry.path(), std::ifstream::binary);

            const std::string shaderCode(
                (std::istreambuf_iterator<char>(shaderFile)),
                std::istreambuf_iterator<char>());

            vk::ShaderModuleCreateInfo shaderCreateInfo(
                {}, static_cast<uint32_t>(shaderCode.size()),
                reinterpret_cast<const uint32_t*>(shaderCode.data()));

            vk::raii::ShaderModule shaderModule{device, shaderCreateInfo};

            shaders.insert(
                {entry.path().filename().replace_extension().string(),
                 std::move(shaderModule)});
        }
    }
    if (shaders.size() == 0)
    {
        LogWarning("No shaders constructed!");
    }
    return shaders;
}

std::optional<vk::raii::ShaderModule>
CompileShader(vk::raii::Device& device, const std::string& shaderName)
{
    std::filesystem::path shader_source_directory =
        std::filesystem::path(WORKING_DIRECTORY).append(std::string("shader"));

    std::filesystem::path shaderPath =
        std::filesystem::path(shader_source_directory).append(shaderName);

    std::vector<uint32_t> shaderCode = CompileShaderFile(shaderPath);

    if (!shaderCode.size())
    {
        return {};
    }

    vk::ShaderModuleCreateInfo shaderCreateInfo(
        {}, static_cast<uint32_t>(shaderCode.size() * sizeof(uint32_t)),
        shaderCode.data());

    return std::optional<vk::raii::ShaderModule>{{device, shaderCreateInfo}};
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

void ReflexShader(const std::vector<uint32_t>& shaderSource)
{
    spv_reflect::ShaderModule module(shaderSource);

    uint32_t count = 0;
    module.EnumerateDescriptorSets(&count, nullptr);

    std::vector<SpvReflectDescriptorSet*> sets(count);
    module.EnumerateDescriptorSets(&count, sets.data());
}