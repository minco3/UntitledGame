#include "Shader.hpp"
#include "spirv_reflect.h"
#include <algorithm>
#include <shaderc/shaderc.hpp>
#include <span>

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

            const std::vector<uint8_t> shaderCode(
                (std::istreambuf_iterator<char>(shaderFile)),
                std::istreambuf_iterator<char>());

            ReflexShader(shaderCode);

            vk::ShaderModuleCreateInfo shaderCreateInfo(
                {}, static_cast<uint32_t>(shaderCode.size()),
                reinterpret_cast<const uint32_t*>(shaderCode.data()));

            vk::raii::ShaderModule shaderModule{device, shaderCreateInfo};

            shaders.emplace(
                entry.path().filename().replace_extension().string(),
                std::move(shaderModule));
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

    const std::vector<uint8_t> shaderCode = CompileShaderFile(shaderPath);

    if (!shaderCode.size())
    {
        return {};
    }

    ReflexShader(shaderCode);

    vk::ShaderModuleCreateInfo shaderCreateInfo(
        {}, static_cast<uint32_t>(shaderCode.size()),
        reinterpret_cast<const uint32_t*>(shaderCode.data()));

    return std::optional<vk::raii::ShaderModule>{{device, shaderCreateInfo}};
}

std::vector<uint8_t> CompileShaderFile(const std::filesystem::path& filePath)
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

    auto compileOptions = shaderc::CompileOptions();
    compileOptions.SetGenerateDebugInfo();

    auto result = c.CompileGlslToSpvAssembly(
        shaderSource, shaderKind, filePath.filename().string().data(),
        compileOptions);

    shaderc_compilation_status status = result.GetCompilationStatus();
    if (status !=
        shaderc_compilation_status::shaderc_compilation_status_success)
    {
        LogWarning(fmt::format(
            "Shader compilation errors: [{}]\n{}", result.GetNumErrors(), result.GetErrorMessage()));
        return {};
    }
    if (result.GetNumWarnings())
    {
        LogWarning(fmt::format(
            "Shader compilation warnings: [{}]\n{}", result.GetNumWarnings(),
            result.GetErrorMessage()));
    }

    return std::vector<uint8_t>(result.cbegin(), result.cend());
}

void ReflexShader(const std::vector<uint8_t>& shaderSource)
{
    spv_reflect::ShaderModule module(shaderSource);

    uint32_t descriptorSetCount = 0;
    module.EnumerateDescriptorSets(&descriptorSetCount, nullptr);

    std::vector<SpvReflectDescriptorSet*> sets(descriptorSetCount);
    module.EnumerateDescriptorSets(&descriptorSetCount, sets.data());

    for (const auto ptr : sets)
    {
        const std::span<SpvReflectDescriptorBinding*> bindings(
            ptr->bindings, static_cast<size_t>(ptr->binding_count));
        for (const auto pbinding : bindings)
        {
            const SpvReflectDescriptorBinding& binding = *pbinding;
            std::cout << binding.block.members << '\n';
        }
    }

    uint32_t inputVariableCount = 0;
    module.EnumerateInputVariables(&inputVariableCount, nullptr);

    std::vector<SpvReflectInterfaceVariable*> inputVariables(inputVariableCount);
    module.EnumerateInputVariables(&inputVariableCount, inputVariables.data());

    for (const auto ptr : inputVariables)
    {
        std::cout << ptr->name << '\n';
    }
}