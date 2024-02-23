#include "Shader.hpp"
#include "spirv_reflect.h"
#include <algorithm>
#include <filesystem>
#include <iostream>
#include <shaderc/shaderc.hpp>
#include <span>

Shader::Shader(
    vk::raii::Device& device, const std::string& _name,
    const vk::ShaderModuleCreateInfo& vertShaderCreateInfo,
    const vk::ShaderModuleCreateInfo& fragShaderCreateInfo)
    : name(_name), vertShaderModule(device, vertShaderCreateInfo),
      fragShaderModule(device, fragShaderCreateInfo)
{
}

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
        // shaderc::Compiler c;
        if (entry.path().extension() == ".spv") // a.frag(.spv)
        {
            std::string name =
                entry.path().stem().stem().string(); // (a).frag.spv
            // check if shader already exists
            if (shaders.contains(name))
            {
                continue;
            }
            
            std::ifstream shaderFile(entry, std::ios::binary);

            std::vector<uint8_t> shaderCode(
                (std::istreambuf_iterator<char>(shaderFile)),
                std::istreambuf_iterator<char>());

            ReflexShader(std::span<const uint32_t>(
                reinterpret_cast<uint32_t*>(shaderCode.data()),
                shaderCode.size()));

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

    const std::vector<uint32_t> shaderCode = CompileShaderFile(shaderPath);

    if (!shaderCode.size())
    {
        return {};
    }

    ReflexShader(
        std::span<const uint32_t>(shaderCode.data(), shaderCode.size()));

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

    auto compileOptions = shaderc::CompileOptions();
    compileOptions.SetGenerateDebugInfo();

    auto result = c.CompileGlslToSpv(
        shaderSource, shaderKind, filePath.filename().string().data(),
        compileOptions);

    shaderc_compilation_status status = result.GetCompilationStatus();
    if (status !=
        shaderc_compilation_status::shaderc_compilation_status_success)
    {
        LogWarning(fmt::format(
            "Shader compilation errors: [{}]\n{}", result.GetNumErrors(),
            result.GetErrorMessage()));
        return {};
    }
    if (result.GetNumWarnings())
    {
        LogWarning(fmt::format(
            "Shader compilation warnings: [{}]\n{}", result.GetNumWarnings(),
            result.GetErrorMessage()));
    }

    return std::vector<uint32_t>(result.cbegin(), result.cend());
}

void ReflexShader(const std::span<const uint32_t>& shaderSource)
{
    spv_reflect::ShaderModule module(shaderSource.size(), shaderSource.data());

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

    std::vector<SpvReflectInterfaceVariable*> inputVariables(
        inputVariableCount);
    module.EnumerateInputVariables(&inputVariableCount, inputVariables.data());

    for (const auto ptr : inputVariables)
    {
        std::cout << ptr->name << '\n';
    }
}
